#include "kt_hexparser.h"
#include "i2ccom.h"
#include<sys/types.h>
#include<sys/time.h>
#include <stdlib.h>

/* Acknowledge message for the client */
#define bl_ack_ack     'A'
#define bl_ack_nack    'N'
#define bl_ack_err     'E'
#define bl_ack_timeout 'T'

#define filename "./PierreTest.hex"
#define port "/dev/i2c/0"
#define i2c_addr 49

i2c_t i2c;

int bl_send_char(char data)
{
  return i2c_llwrite( &i2c , i2c_addr, &data, 1);
}

char bl_calc_crc(char * data, unsigned int length)
{
  unsigned int i;
  char crc=0;

#if 0
  for(i=0; i<length;i++)
    printf("%c.",data[i]);
  printf("\r\n");
#endif

  for(i=0; i<length;i++)
    crc+=data[i];

  return crc;
}
int bl_prepare_frame(char * frame,int code, int datacount, int addr, char * data)
{
  int i;

  switch(code)
  {
    case 0:
      frame[0] = code;
      frame[1] = datacount;
      frame[2] = (addr >> 8) & 0x00FF;
      frame[3] = addr & 0x00FF;

      for(i=0; i<datacount;i++)
	frame[4+i] = data[i];

      return (4+datacount);
      break;

    case 1:
      frame[0] = code;
      frame[1] = datacount;
      return 2;
      break;
      
    case 4:
      frame[0] = code;
      frame[1] = data[0];
      frame[2] = data[1];

      return 3;
      break;

    default:
      return 0;
      break;
  }
}

int bl_send_frame(char * data,int datacount)
{
  int i;

  /* Send the frame length */
  bl_send_char(datacount+2);	

  /* Send a data frame */
  for(i=0; i<datacount; i++)
    bl_send_char(data[i]);

  /* Sending the crc */
  bl_send_char(bl_calc_crc(data,datacount));

  /* Send the endchar */
  bl_send_char('\n');
}

char bl_get_ack()
{
  struct timeval lastcommand, now, diff;
  struct timezone here;
  char buf[2];

  gettimeofday(&lastcommand,&here);

  buf[0] = 'W';
  while(buf[0] == 'W')
  {
    gettimeofday(&now,&here);
    timersub(&now,&lastcommand,&diff);

    /* Timeout */
    if(diff.tv_sec >= 10)
      return bl_ack_timeout;
    /* Read a char from the target */
    /* This call must be non blocking */
    i2c_llread( &i2c, i2c_addr, buf, 1);
  }
  return buf[0];
}


int main(int argc, char **argv)
{
  int i,rc;
  char ack;

  char bl_current_frame[32];
  int  bl_current_framelen;

  enum {bl_state_end, bl_state_waitack, bl_state_data, bl_state_resend, bl_state_restart} bl_client_state;

  pthread_t echo_task;

  FILE* progfile;

  int bl_frame_code;
  int bl_frame_addr;
  int bl_frame_count;
  char bl_frame_data[16];

  printf("K-Team uC Programmer v1.0\r\n");
  
  /* Open Serial port */
  rc = i2c_open(&i2c, port);
  if(rc<0)
  {
    fprintf(stderr,"Cannot open i2c port (%s) aborting\r\n", port);
    exit(0);
  }
  
  if(argv[1] != NULL)
  {
	/* Open program file */
  	progfile = fopen(argv[1],"r");
  	if(!progfile)
  	{
    		fprintf(stderr,"Cannot find %s aborting\r\n", argv[1]);
    		exit(0);
  	}

  	/* Check hex file content */
  	if(hexp_valid_hex(progfile)<0)
  	{
    		fprintf(stderr,"Invalid Hex File aborting\r\n");
    		exit(0);
  	}
  
  	bl_frame_data[0] = 2;
  	bl_frame_data[1] = 'U';
  	
	i2c_llwrite( &i2c, i2c_addr, bl_frame_data, 2);

  	hexp_start(progfile);
  	bl_client_state  = bl_state_data;

  	while(bl_client_state != bl_state_end)
  	{
    		switch(bl_client_state)
    		{
      			case bl_state_data :
				rc = hexp_next_line(progfile,&bl_frame_code,&bl_frame_count,&bl_frame_addr,bl_frame_data);
				if(rc == 1)
				{
	  				/* Format frame to send */
	  				bl_current_framelen = bl_prepare_frame(bl_current_frame,bl_frame_code,bl_frame_count,bl_frame_addr,bl_frame_data);
	  				//printf("sending: code:%d length:%d addr:0x%x\n",bl_frame_code,bl_frame_count,bl_frame_addr);
	  				/* Send a frame */
	  				bl_send_frame(bl_current_frame,bl_current_framelen);
	  				bl_client_state = bl_state_waitack;
				}
				else
					if(rc == 2)  
	  					bl_client_state = bl_state_data;
					else
	  					bl_client_state = bl_state_end;
				break;

      			case bl_state_waitack :
				/* Wait for the target answer */
				ack = bl_get_ack();
				switch(ack)
				{
	  				case bl_ack_ack:
	    					/* ACK received, next frame */
	    					//printf("ack\r\n");
	   					bl_client_state = bl_state_data;
	    					break;
	  				case bl_ack_nack:
	    					/* NACK received, resend frame */
	    					//printf("nack\r\n");
	    					bl_client_state = bl_state_resend;
	    					break;
	 	 			case bl_ack_err:
	    					/* ERR received, giveup */
	    					//printf("err\r\n");
	    					bl_client_state = bl_state_resend;
	   					break;
	 	 			case bl_ack_timeout:
	    					/* Timeout waiting target answer, giveup */
	    					printf("Timeout Receiving\r\n");
	    					bl_client_state = bl_state_end;
	   					break;
	  				default:
	    					/* Unknown answer, ask the target to restart programming */
	    					//printf("unknown(0x%x)\r\n",ack);
	    					bl_client_state = bl_state_resend;
	   					 break;
				}
				break;

      			case bl_state_resend :
				/* Resend the same frame */
				//printf("re-sending: code:%d length:%d addr:0x%x\n",bl_frame_code,bl_frame_count,bl_frame_addr);
				bl_send_frame(bl_current_frame,bl_current_framelen);
				bl_client_state = bl_state_waitack;
				break;
		}	
  	}
  
  	fprintf(stderr,"Programming Terminated\r\n");

  	/* Program Checking Phase */	
#if 0
  	while(1)
  	{
  	}
#endif

  	i2c_close(&i2c);
  	fclose(progfile);
 }
 else
 {
	printf("Usage : ucbootload myhex.hex\r\n");
 }
}
