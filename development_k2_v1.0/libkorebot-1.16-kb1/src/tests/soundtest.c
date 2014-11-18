#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <korebot/korebot.h>

int main(int argc, char * argv[])
{
   WAV wav;
   int rc;
   snd_t snd;
   short *buf;
   unsigned int size = 2048;
   
   /* Checking args */
   if(argc < 2)
   {
     printf("usage: soundtest <file.wav>\r\n");
     return 1;
   }

   buf = kb_alloc( size );

   kb_snd_init(&snd , "/dev/sound/dsp", "/dev/sound/mixer");

   if ( kb_snd_open(&snd ) < 0 ) {
     printf("cannot open sound devices\r\n");
     return 1;
   }

   kb_wav_init( &wav );
   
   if ((rc = kb_wav_open( &wav , argv[1], 1 )) == -1 ) {
     printf("cannot open %s\r\n",argv[1]);
     return 1;
   }

   if ( wav.fmt.bit_per_sample != 16  && wav.fmt.nb_channel != 2 ) {
     printf("wrong bit_per_sample or nb_channel in wav file\r\n");
     return 1;
   }

   /* Set Sample Rate */
   if ( kb_snd_setSampleRate( &snd, wav.fmt.sample_rate ) == -1 ) {
     printf("unable to set sample rate\r\n");
     return 1;
   }

   for (;;) {
     rc = kb_wav_read_data( &wav , buf , size);
     if ( rc <= 0 ) {
       if ( rc < 0 )
	 printf( "i/o error in reading file '%s' !" , argv[1]);
       break;
     }

     do {
       rc = kb_snd_play( &snd , buf , size );
       if ( rc < 0 ) {
	 if ( errno != EAGAIN ) {
	   printf( "error in playing sample errno=%d", errno );
	   break;
	 }
	 continue;
       }
     }
     while (0);
   }

   kb_free( buf );
   kb_wav_close( &wav );
}
