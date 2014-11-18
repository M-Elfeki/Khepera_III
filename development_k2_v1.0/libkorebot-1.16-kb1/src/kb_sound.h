/*--------------------------------------------------------------------*/

#ifndef __sound_h__
#define __sound_h__

#include <linux/soundcard.h>

typedef struct {

  char dsp_device[ PATH_MAX + 1 ];
  char mixer_device[ PATH_MAX + 1 ];
  
  int dsp_fd; 
  int mix_fd;
  unsigned int sampleSize;
  unsigned int sampleRate;
  unsigned int nbChannels; 
  unsigned int bytePerSample;
  
  short      * ibuf;
  short      * iend;
  unsigned int isize; /* in bytes */

  short      * obuf;
  short      * oend;
  unsigned int osize; /* in bytes */

  unsigned int fragSize;
  unsigned int fragCount;
}
snd_t;

/*--------------------------------------------------------------------*/

extern void kb_snd_init( snd_t * snd ,
		      const char * mixer_device ,
		      const char * dsp_device );

extern void kb_snd_exit( snd_t * snd );

extern int kb_snd_open( snd_t * snd );

extern void kb_snd_close( snd_t * snd );

extern int kb_snd_doMmap( snd_t * snd );

extern int kb_snd_doMunmap( snd_t * snd );

extern int kb_snd_enableTrigger( snd_t * snd , int mask );

extern int kb_snd_enableFrag( snd_t * snd );

extern short * kb_snd_getIPtr( snd_t * snd );

extern short * kb_snd_getOPtr( snd_t * snd );

extern unsigned int kb_snd_getFragSize( snd_t * snd );

extern void kb_snd_copyFragTo( snd_t * snd , short * buf );

extern void kb_snd_copyFragFrom( snd_t * snd , short * buf );

extern int kb_snd_setVolume( snd_t * snd , 
		   unsigned int line , 
		   unsigned int vol_left ,
		   unsigned int vol_right );

extern int kb_snd_setSampleRate( snd_t * snd ,
			      unsigned int sampleRate );

extern int kb_snd_waitFrag( snd_t * snd );

extern int kb_snd_record( snd_t * snd , 
		       short * data , unsigned long nsamples );

extern int kb_snd_play( snd_t * snd , 
		     short * data , unsigned long nsamples );

extern int kb_snd_reset( snd_t * snd );

extern int kb_snd_flush( snd_t * snd );

extern int kb_snd_openDevices( snd_t *snd );

extern unsigned long kb_smplfromdur(unsigned int duration, unsigned smplrate);

extern unsigned long kb_durfromsmpl(unsigned int smplnbr, unsigned smplrate);

extern unsigned int kb_snd_fragInit(snd_t *snd, int line1Vol, int line2Vol, int smplRate, int mode);

extern void kb_snd_fragPlay(snd_t *snd, int fragSize);

/*--------------------------------------------------------------------*/

#endif /* __sound_h__ */
