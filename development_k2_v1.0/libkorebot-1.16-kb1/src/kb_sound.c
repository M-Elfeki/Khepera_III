#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "korebot.h"

/*--------------------------------------------------------------------*/
void kb_snd_init( snd_t * snd ,
	      const char * dsp_device ,
	      const char * mixer_device )
{
  strncpy( snd->dsp_device , dsp_device , PATH_MAX );
  strncpy( snd->mixer_device , mixer_device , PATH_MAX );
  snd->dsp_fd = -1;
  snd->mix_fd = -1;
  snd->ibuf = NULL;
  snd->iend = NULL;
  snd->obuf = NULL;
  snd->oend = NULL;
  snd->fragSize = 0;
  snd->fragCount = 0xFFFF;
}

/*--------------------------------------------------------------------*/
/*!
 */
void kb_snd_exit( snd_t * snd )
{
  /* unmmap */
  kb_snd_close( snd );
}

/*--------------------------------------------------------------------*/
/*! This function opens all devices need for producing sound. 
 */
int kb_snd_open( snd_t * snd )
{
  int format;
  int stereo;
  audio_buf_info info;
  
  if ( snd->mix_fd == -1 ) {
    
    if ((snd->mix_fd = open( snd->mixer_device , O_RDWR )) < 0 ) {
      perror(snd->mixer_device);
      kb_snd_close( snd );
      return -1;
    }
  }


  if ( snd->dsp_fd == -1 ) {

    //NB: Why this would be opened in non blocking mode???
    //if ((snd->dsp_fd = open( snd->dsp_device , O_NDELAY|O_RDWR )) < 0 ) {
    if ((snd->dsp_fd = open( snd->dsp_device , O_RDWR )) < 0 ) {
      perror(snd->dsp_device);
      return -1;
    }
  }

  snd->sampleSize = 16;
  format = AFMT_S16_LE;

  if (ioctl( snd->dsp_fd , SNDCTL_DSP_SETFMT, &format ) < 0 ) {
    return -1;
  }

  if ( format != AFMT_S16_LE ) {
    fprintf(stderr, "Bad format\n");
    return -1;
  }

  stereo = 1;

  if ( ioctl( snd->dsp_fd , SNDCTL_DSP_STEREO , &stereo ) < 0 ) {
    return -1;
  }

  if ( stereo != 1 ) {
    fprintf(stderr, "Not stereo\n");
    return -1;
  }

  snd->nbChannels = 2; /* stereo */
  
  if ( ioctl( snd->dsp_fd , 
	      SOUND_PCM_READ_CHANNELS , 
	      &snd->nbChannels ) < 0 ) {
    return -1;
  }

  if ( ioctl( snd->dsp_fd , SNDCTL_DSP_GETOSPACE , &info ) < 0 ) 
    return -1;

  snd->bytePerSample = snd->nbChannels * ( snd->sampleSize / 8 ); 

  snd->fragSize = info.fragsize;
  //snd->fragSize = (1<<14);
  snd->fragCount = info.fragstotal;
  KB_DEBUG("snd_open", 0, "fragSize=%u fragCount=%u" ,
              snd->fragSize , snd->fragCount );

  return 1;
}

/*--------------------------------------------------------------------*/
/*! This function closes the file descriptor opened by kb_snd_open
 */
void kb_snd_close( snd_t * snd )
{
  if ( snd->dsp_fd != -1 ) {
    close( snd->dsp_fd );
    snd->dsp_fd = -1;
  }

  if ( snd->mix_fd != -1 ) {
    close( snd->mix_fd );
    snd->mix_fd = -1;
  }

  if ( snd->ibuf != NULL ) {
    kb_snd_doMunmap( snd );
  }
}

/*--------------------------------------------------------------------*/
/*!
 */
int kb_snd_doMmap( snd_t * snd )
{
  unsigned int size = snd->fragSize * snd->fragCount;

  KB_DEBUG( "snd_doMmap", 0, "map size = %u" , size );

  if ( snd->ibuf != NULL ) return 0;

  if ( snd->dsp_fd >= 0 ) {
  
    if ((snd->ibuf = mmap( NULL ,  size , PROT_READ , 
			   MAP_FILE|MAP_SHARED ,
			   snd->dsp_fd , 0 )) == (void *)-1) {
      return -1;
    }
  }

  snd->iend = (short *)((unsigned char *)snd->ibuf + size);
  snd->isize = size;

  KB_DEBUG( "snd_doMmap", 0, "ibuf=0x%08X isize=0x%08X" , 
  	       snd->ibuf , snd->isize );

  if ( snd->dsp_fd >= 0 ) {

    if ((snd->obuf = mmap( NULL , size , PROT_WRITE ,
			   MAP_FILE|MAP_SHARED ,
			   snd->dsp_fd , 0 )) == (void *)-1) {
      return -1;
    }
  }

  snd->oend = (short *)((unsigned char *)snd->obuf + size) ;
  snd->osize = size;

  KB_DEBUG( "snd_doMmap", 0, "obuf=0x%08X osize=0x%08X" ,
	       snd->obuf , snd->osize );

  return 1;
}

/*--------------------------------------------------------------------*/
/*!
 */
int kb_snd_doMunmap( snd_t * snd )
{
  if ( snd->ibuf != NULL ) {
    munmap( snd->ibuf , snd->isize );
    snd->ibuf = NULL;
    snd->iend = NULL;
    snd->isize = 0;
  }

  if ( snd->obuf != NULL ) {
    munmap( snd->obuf , snd->osize );
    snd->obuf = NULL;
    snd->oend = NULL;
    snd->osize = 0;
  }

  return 0;
}
/*--------------------------------------------------------------------*/

int kb_snd_enableTrigger( snd_t * snd , int mask )
{
  int trig = 0;

  if ( snd->dsp_fd < 0 )
    return -1;
  
  if (ioctl( snd->dsp_fd , SNDCTL_DSP_SETTRIGGER , &trig ) < 0)
      return -1;

  if ( trig != 0 ) {
    fprintf( stderr , "Bad trigger (0x%08X)\n" , trig );
    return -1;
  }

  trig = mask;

  if ( ioctl( snd->dsp_fd , SNDCTL_DSP_SETTRIGGER , &trig ) < 0 )
    return -1;

  if ( trig != mask ) {
    fprintf( stderr , "Bad trigger (0x%08X!=0x%08X)\n" , 
	     trig , mask );
    return -1;
  }

  KB_DEBUG( "snd_enableTrigger", 0, "Enable Trigger Ok");

  return 0;
}

/*--------------------------------------------------------------------*/
/*!
 */
int kb_snd_enableFrag( snd_t * snd  )
{
  int frag;
  int pos;
  int fragExponent = 0;

  if ( snd->dsp_fd < 0 ) 
    return -1;

  for (pos=0; pos<32; pos++) {
    if ( snd->fragSize & (1<<pos)) {
      fragExponent = pos;
      break;
    }
  }
  
  KB_DEBUG( "snd_enableFrag", 0, "fragExponent = %u" , fragExponent );

  frag = ( snd->fragCount << 16 ) | ( fragExponent );

  if ( ioctl( snd->dsp_fd , SNDCTL_DSP_SETDUPLEX , 0 ) < 0 ) {
    KB_ERROR("snd_enableFrag", KB_ERROR_DUPLEX);
    return -1;
  }

  if ( ioctl( snd->dsp_fd , SNDCTL_DSP_SETFRAGMENT , &frag ) < 0 ) {
    KB_ERROR("snd_enableFrag", KB_ERROR_FRAGMENT );
    return -1;
  }
 
  if (( frag & 0xFFFF) != fragExponent ) {
    KB_ERROR("snd_enableFrag",KB_ERROR_FRAGEXP , fragExponent , (frag&0xffff)); 
    return -1;
  }

  KB_DEBUG( "snd_enableFrag", 0, "EnableFragment Ok");

  return 1;
}

/*--------------------------------------------------------------------*/

short *kb_snd_getIPtr( snd_t * snd )
{
  count_info info;
  unsigned char *ptr;

  if ( snd->dsp_fd < 0 )
    return NULL;
  
  if ( ioctl( snd->dsp_fd , SNDCTL_DSP_GETIPTR , &info ) < 0 ) {
    KB_ERROR( "snd_getIPtr", KB_ERROR_IPTR );
    return NULL;
  }
  
  ptr = (unsigned char *)snd->ibuf;
  ptr+= info.ptr;
  ptr-= snd->fragSize;
  if ( ptr < (unsigned char *)snd->ibuf ) {
    ptr += snd->isize;
  }
 
  return (short *)ptr;
}

/*--------------------------------------------------------------------*/

short *kb_snd_getOPtr( snd_t * snd )
{
  unsigned char * ptr;
  count_info info;

  if ( snd->dsp_fd < 0 )
    return NULL;

  if ( ioctl( snd->dsp_fd , SNDCTL_DSP_GETOPTR , &info ) < 0 ) {
    KB_ERROR( "snd_getOPtr", KB_ERROR_OPTR );
    return NULL;
  }

  ptr = (unsigned char *)snd->obuf;
  ptr += info.ptr;
  ptr += snd->fragSize;
  if ( ptr >= (unsigned char *)snd->oend ) {
    ptr -= snd->osize;
  }
   
  return (short *)ptr;

}

/*--------------------------------------------------------------------*/
/*!
 */
unsigned int kb_snd_getFragSize( snd_t * snd )
{
  return snd->fragSize;
}

/*--------------------------------------------------------------------*/
/*!
 */
void kb_snd_copyFragTo( snd_t * snd , short * buf ) 
{
  short * ptr;

  ptr = (short*)kb_snd_getIPtr(snd);
  if ( ptr != NULL ) {
    memcpy( buf , ptr , snd->fragSize );
  }
}

/*--------------------------------------------------------------------*/
/*!
 */
void kb_snd_copyFragFrom( snd_t * snd , short * buf )
{
  short * ptr;

  ptr = (short*)kb_snd_getOPtr( snd );
  if ( ptr != NULL ) {
    memcpy( ptr , buf , snd->fragSize );
  }
}

/*--------------------------------------------------------------------*/
/*! This function sets the input/output volume and selects 
 *  the input.
 */
int kb_snd_setVolume( snd_t * snd , 
		   unsigned int line , 
		   unsigned int vol_left ,
		   unsigned int vol_right )
{
  int rc = -1;
  int arg = ((vol_left<<8)|vol_right);
  
  if ( snd->mix_fd >= 0 ) {
    rc = ioctl( snd->mix_fd , MIXER_WRITE(line) , &arg ); 
  }
  return rc;
}

/*--------------------------------------------------------------------*/
/*! This function sets the sampling rate
 */
int kb_snd_setSampleRate( snd_t * snd , unsigned int sampleRate )
{
  int rc = -1;

  KB_DEBUG( "snd_setSampleRate", 0, "SampleRate=%u" , sampleRate );
  
  if ( snd->dsp_fd >= 0 ) {
    rc = ioctl( snd->dsp_fd , SNDCTL_DSP_SPEED , &sampleRate );
  }
  return rc;
}

/*--------------------------------------------------------------------*/
/*!
 */
int kb_snd_waitFrag( snd_t * snd ) 
{
  fd_set set;
  int rc = -1;

  if ( snd->dsp_fd < 0 )
    return -1;

  FD_ZERO(&set);
  FD_SET( snd->dsp_fd , &set);
  while (1) {

    if ((rc=select( snd->dsp_fd+1 , NULL , &set , NULL, NULL )) < 0 ) {
      if ( errno == EINTR ) continue;
    }
    return rc;
  }
}

/*--------------------------------------------------------------------*/
/*!
 */
void kb_snd_sync( snd_t * snd )
{
  if ( snd->dsp_fd >= 0 ) 
    ioctl( snd->dsp_fd , SNDCTL_DSP_SYNC , NULL );
}

/*--------------------------------------------------------------------*/
/*! This function records a given number of samples
 */
int kb_snd_record( snd_t * snd , short * data , unsigned long size )
{
  int rc = -1; 
  
  if ( snd->dsp_fd >= 0 ) {
    rc = read( snd->dsp_fd , (void *)data , size );
    
    if ( rc < 0 ) {
      if ( errno == EAGAIN ) 
	rc = 0;
    }
  }
 
  return rc;
}

/*--------------------------------------------------------------------*/
/*! This function plays a given number of samples
 */
int kb_snd_play( snd_t * snd , short * data , unsigned long size )
{
  int rc = 0;

 if ( snd->dsp_fd >= 0 ) {
    /*snd_reset( snd );*/

    rc = write( snd->dsp_fd , (void *)data , size );
    
    kb_snd_flush( snd );

  }
  return rc;
}

/*--------------------------------------------------------------------*/

int kb_snd_flush( snd_t * snd )
{
  int rc = -1;
  if ( snd->dsp_fd >= 0 ) 
    rc=ioctl( snd->dsp_fd , SNDCTL_DSP_POST);
  return rc;
}

/*--------------------------------------------------------------------*/

int kb_snd_reset( snd_t * snd )
{
  int rc = -1;
  if ( snd->dsp_fd >= 0 )
    rc=ioctl( snd->dsp_fd , SNDCTL_DSP_RESET );
  return rc;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function open and initializes the default sound devices
 *
 * \param snd pointer to a snd_t structure 
 * \return an error code
 *        - 1 error
 *        - 0 no error
 */
int kb_snd_openDevices( snd_t *snd )
{
  int rc;
  
  /* Initialize the korebot library return with 1 if any issue */
  if((rc= kb_init( 0, NULL )) < 0 )
    return 1;  
    
  KB_DEBUG( "kb_openDevices", 0, "kb_init Ok");
  
  /* Initialize the korebot sound devices */
  kb_snd_init( snd , "/dev/sound/dsp" , "/dev/sound/mixer" );
  
  /* open the sound device return with 1 if any issue */
  if (kb_snd_open(snd) < 0 ) {
    fprintf(stderr , "unable to open sound device\n");
    return (1);
  }
    
  KB_DEBUG( "kb_openDevices", 0, "kb_snd_init Ok");
  /* return with success */
  return 0;
}

/*--------------------------------------------------------------------*/
/*! 
 * This function get the number of samples for a given duration
 *
 * \param duration	duration in second
 * \param smplrate	sample rate ie 44100 
 * \return the number of samples for a given duration
 */
unsigned long kb_smplfromdur(unsigned int duration, unsigned smplrate)
{
  return duration*smplrate*2;
}


/*--------------------------------------------------------------------*/
/*! 
 * This function get the duration for a given sample number
 *
 * \param smplnbr	duration in second 
 * \param smplrate	sample rate ie 44100
 * \return the duration
 */
unsigned long kb_durfromsmpl(unsigned int smplnbr, unsigned smplrate)
{
  return smplnbr/smplrate/2;
}


/*--------------------------------------------------------------------*/
/*! 
 * This function completely initialise then open the sound devices
 * various volumes such as the "Line IN", sample rate and mode are
 * set from the various arguments
 *
 * \param snd 	pointer to a snd_t structure
 * \param line1Vol	"Line 1" volume ( from 0 to 255 )
 * \param line2Vol	"Line 2" volume ( from 0 to 255 ) 
 * \param smplrate	sample rate, ie 44100
 * \param mode	Trigger mode :
 *  - 0 Input way
 *  - 1 Outptut way
 *  - 2 both way 
 * \return frag size :
 *  - 0 any error
 *  - > 0 fragment size
 */
unsigned int kb_snd_fragInit(snd_t *snd, int line1Vol, int line2Vol, int smplRate, int mode)
{
  int frag = 0;
  int ictl;
  
  /* Decide which trigger type might be used */
  switch(mode)
  {
    case 0 : ictl = PCM_ENABLE_INPUT; /* only input */
    	     break;
    case 1 : ictl = PCM_ENABLE_OUTPUT; /* only output */
    	     break; 
    default: ictl = PCM_ENABLE_INPUT | PCM_ENABLE_OUTPUT; /* both input - output */	     
  }
  
  if(!kb_snd_openDevices(snd)){
    
  
    /* set the sample rate from the argument */
    kb_snd_setSampleRate( snd , smplRate ); 
  
    /* decide the "line in" volume */
    kb_snd_setVolume( snd , SOUND_MIXER_LINE1 , line1Vol, line1Vol ); 
    kb_snd_setVolume( snd , SOUND_MIXER_LINE2 , line2Vol, line2Vol ); 
  
    /* enable fragment acquiring type and map it */
    kb_snd_enableFrag( snd );
    kb_snd_doMmap( snd );
    
    /* set the trigger from a given mode input argument */
    kb_snd_enableTrigger( snd , ictl ); 
  
    /* retrieve the fragment size from environment */
    frag = kb_snd_getFragSize( snd );
  } 
  
  /* return 0 if error, else the fragment size */
    return frag;
}


/*--------------------------------------------------------------------*/
/*! 
 * This function just play a fragment from the sound card
 * both argument supposed to be retrieved with the kb_snd_fragInit() 
 *
 * \param snd 	pointer to an initialised snd_t structure
 * \param fragSize	fragment size
 */
void kb_snd_fragPlay(snd_t *snd, int fragSize)
{
  short *in , *out;
  
  /* Wait till the next fragment has been fullfilled */  
  kb_snd_waitFrag( snd );
  
  /* Retrieve in(Lines) and out(Out) pointers */
  in = kb_snd_getIPtr( snd );
  out = kb_snd_getOPtr( snd );
  
  /* Copy the fragment from receive buffer to the transmit buffer */
  memcpy( out , in , fragSize ); //! just do some OutFrag = InFrag
}

