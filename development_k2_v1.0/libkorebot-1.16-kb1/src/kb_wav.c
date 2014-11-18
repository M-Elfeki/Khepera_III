/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2004/11/12 14:35:23 $
 * $Revision: 1.3 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/kb_wav.c,v 1.3 2004/11/12 14:35:23 pbureau Exp $
 */

/*--------------------------------------------------------------------*/
/*! 
 * \file   kb_wav.c wav files handling
 *
 * \brief
 *         This module provides functions to read, write and playback
 *         wav data from/to a wav file.
 *
 * \author   Cédric Gaudin (K-Team SA)
 * \author   Pierre Bureau (K-Team SA)
 * \note     Copyright (C) 2004 K-TEAM SA
 */

#include "korebot.h"

#define print_type(t)  printf("%c%c%c%c\n" , t[0], t[1], t[2], t[3])

/*--------------------------------------------------------------------*/
/*! Play a wav file to the given sound interface. The sound interface 
 * must be initialized first using kb_sound_init and kb_sound_open.
 * This function does not return until the full wav file is played.
 * \param snd		the sound interface
 * \param filename	the name of wav file to play
 * \param soundbufsize	the size of the sound buffer
 * \return an error code or 0 if successful
 */
int kb_wav_play(snd_t * snd, char * filename, unsigned soundbufsize)
{
  WAV wav;
  int rc;
  short buf[soundbufsize];

  /* Init the wav struct */
  kb_wav_init( &wav );

  /* Open the given wav file */
  if ((rc = kb_wav_open( &wav , filename, 1 )) == -1 ) {
    return KB_ERROR("kb_wav_play",KB_ERROR_FILEOPEN, filename);
  }

  /* Check that the format is supported */
  /*! \todo change the format check to be used with different sound interfaces. So far only the KoreMedia interface is supported.
  */
  if ( wav.fmt.bit_per_sample != 16  && wav.fmt.nb_channel != 2 ) {
    return KB_ERROR("kb_wav_play",KB_ERROR_BADSOUND, filename);
  }

  /* Sound playing loop */
  for (;;) {
    rc = kb_wav_read_data( &wav , buf , soundbufsize);
    if ( rc <= 0 ) {
      if ( rc < 0 )
	kb_warning(KB_WARN_SOUNDIO , filename);
      break;
    }

    do {
      rc = kb_snd_play( snd , buf , soundbufsize);
      if ( rc < 0 ) {
	if ( errno != EAGAIN ) {
	  kb_warning(KB_WARN_SOUNDSAMPLE, errno);
	  break;
	}
	continue;
      }
    }
    while (0);
  }

  /* Close the wav file */
  kb_wav_close( &wav );

  return 0;
}

/*--------------------------------------------------------------------*/
/*! This function initializes the wav struct, it must be called before 
 *  calling other wav functions.
 *  \param wav	the wav struct to initialize
 */
void kb_wav_init( WAV * wav ) 
{
  wav->file = NULL;
  wav->length = 0;
  wav->modified = 0;
}

/*--------------------------------------------------------------------*/
/*! This function opens a wav file for reading or writing
 * \param wav	the wav struct to use with the given file
 * \param file	the file to read or write
 * \param read	set to 1 for reading or to 0 for writing
 * \return 
 * 	- the data length on success
 * 	- -1 if the file cannot be opened
 */
int kb_wav_open( WAV * wav , const char * file , int read )
{
  COMMON_CHUNK_HEADER hdr;
  int type;
    
  if ((wav->file = fopen( file , read ? "rb" : "wb" )) == NULL) {
    return -1;
  }
  
  if ( read ) {
    for (;;) {
      type = kb_wav_read_hdr( wav , &hdr );
      switch( type ) {
        /* RIFF */
        case 1:
          break;
        
        /* FMT */
        case 2:
          memcpy( &wav->fmt , &hdr.chunk.fmt , sizeof(wav->fmt));
          break;
        
        /* DATA */
        case 3:
          wav->length = hdr.length;
          return (int)hdr.length;
      }
    }
  }
  else {
    memcpy( &hdr.type , "RIFF" , 4 );
    hdr.length = sizeof( RIFF_CHUNK_HEADER) + 8 + 
      sizeof( FMT_CHUNK_HEADER) + 8 + 8 + wav->length;
    memcpy( &hdr.chunk.riff.chunk_content , "WAVE" , 4 );
    kb_wav_write_hdr( wav , &hdr );
    
    memcpy( &hdr.type , "fmt " , 4 );
    hdr.length = sizeof( FMT_CHUNK_HEADER ) ;
    memcpy( &hdr.chunk.fmt , &wav->fmt , sizeof( FMT_CHUNK_HEADER ));
    kb_wav_write_hdr( wav , &hdr );
    
    memcpy( &hdr.type , "data" , 4 );
    hdr.length = wav->length;
    kb_wav_write_hdr( wav , &hdr );
    wav->modified = 0;
  }
  
  return 0;
}

/*--------------------------------------------------------------------*/
/*! This function closes a wav file.
 * \param wav	the wav struct linked to the wav file 
 */
void kb_wav_close( WAV * wav )
{
  COMMON_CHUNK_HEADER hdr;
  
  if ( wav->modified ) {
    rewind( wav->file );
    memcpy( &hdr.type , "RIFF" , 4 ); 
    hdr.length = sizeof( RIFF_CHUNK_HEADER) + 
      sizeof( FMT_CHUNK_HEADER) + 8 + 8 + wav->length;
    memcpy( &hdr.chunk.riff.chunk_content , "WAVE" , 4 );
    kb_wav_write_hdr( wav , &hdr );
    
    memcpy( &hdr.type , "fmt " , 4 );
    hdr.length = sizeof( FMT_CHUNK_HEADER ) ;
    memcpy( &hdr.chunk.fmt , &wav->fmt , sizeof( FMT_CHUNK_HEADER ));
    kb_wav_write_hdr( wav , &hdr );
    
    memcpy( &hdr.type , "data" , 4 );
    hdr.length = wav->length;
    kb_wav_write_hdr( wav , &hdr );
    wav->modified = 0;
  }

  fclose(wav->file);
  wav->file = NULL;
}

/*--------------------------------------------------------------------*/
/*! This function reads a given number of data from a file opened with 
 *  kb_wav_open. The resulting data are placed in buf and can be played 
 *  using kb_snd_play.
 *  \param wav	the wav struct
 *  \param buf	the buffer to store the data
 *  \param size	the length of data to read
 *  \return the number of bytes read
 */
int kb_wav_read_data( WAV * wav , 
                   void * buf , unsigned int size )
{
  int n;
  
  n=fread( buf , sizeof(unsigned char) , size , wav->file );
  if ( n>0 ) {
#if BYTE_ORDER == BIG_ENDIAN
    unsigned int pos;
    unsigned int incr;
    unsigned int nsample;
    unsigned short *p16;
    unsigned long *p32;
   
    incr = wav->fmt.byte_per_sample;
    nsample = size / incr;
    p16 = (unsigned short *)buf;
    p32 = (unsigned long *)buf; 
    for (pos=0; pos<nsample; pos++) {
      switch(incr) {
        case 2:
          p16[pos] = SWAP_WORD(p16[pos]);
          break;

        case 4:
          p32[pos] = SWAP_LONG(p32[pos]);
          break;
      }
    }
#endif /* BYTE_ORDER == BIG_ENDIAN */
  }
  return n;
}

/*--------------------------------------------------------------------*/
/*! This function writes some data into the file.
 * \param wav	the wav struct
 * \param buf	the data to be writen 
 * \param len	the amount of bytes to write
 * \return the number of bytes writen
 */
int kb_wav_write_data( WAV * wav ,
                    void * buf , unsigned int len )
{
  int n;

#if BYTE_ORDER == BIG_ENDIAN
  unsigned int pos;
  unsigned int incr;
  unsigned int nsample;
  unsigned short * p16;
  unsigned long *p32;
  
  incr = wav->fmt.byte_per_sample;
  nsample = len / incr;
  p16 = (unsigned short *)buf;
  p32 = (unsigned long *)buf;
  
  for (pos=0; pos<nsample; pos++) {
    switch(incr) {
      case 2:
        p16[pos] = SWAP_WORD(p16[pos]);
        break;                             
          
      case 4:
        p32[pos] = SWAP_LONG(p32[pos]);
        break;
    }
  }
#endif /* BYTE_ORDER == BIG_ENDIAN */
  n=fwrite( buf , sizeof(unsigned char) , len , wav->file );
  if ( n > 0 ) {
    wav->length += n;
    wav->modified = 1;
  }

  return n;
}


/*--------------------------------------------------------------------*/
/*! Set the format for the given wav struct. The format is usually read
 * from the wav file header using kb_wav_open.
 */
void kb_wav_set_format( WAV * wav ,
                     unsigned short format ,
                     unsigned short nb_channel ,
                     unsigned long sample_rate ,
                     unsigned short bit_per_sample )
{
  unsigned long byte_per_sec;
  unsigned short byte_per_sample;
  
  wav->fmt.format          = format;
  wav->fmt.nb_channel      = nb_channel;
  wav->fmt.sample_rate     = sample_rate;
  wav->fmt.bit_per_sample  = bit_per_sample;

  byte_per_sample = ( bit_per_sample / 8 );
  byte_per_sec = ( nb_channel * byte_per_sample * sample_rate );

  wav->fmt.byte_per_sample = byte_per_sample;
  wav->fmt.byte_per_sec    = byte_per_sec; 
  wav->modified = 1;
}

/*--------------------------------------------------------------------*/
/*! This function reads Wave File header.
 */
int kb_wav_read_hdr( WAV * wav , COMMON_CHUNK_HEADER * hdr )
{
  fread( (void *)hdr , 8  , 1 , wav->file );

  if ( memcmp( hdr->type , "RIFF" , 4 )==0 ||
       memcmp( hdr->type , "riff" , 4 )==0) {
    
    fread( (void *)&hdr->chunk.riff , sizeof(RIFF_CHUNK_HEADER) , 1 , 
           wav->file );

#if BYTE_ORDER == BIG_ENDIAN
    hdr->length = SWAP_LONG(hdr->length);
#endif /* BYTE_ORDER == BIG_ENDIAN */
    return 1;
  }

  if ( memcmp( hdr->type , "FMT " , 4 )==0 ||
       memcmp( hdr->type , "fmt " , 4 )==0) {
    fread( (void *)&hdr->chunk.fmt , sizeof(FMT_CHUNK_HEADER) , 1 ,
           wav->file );
    
#if BYTE_ORDER == BIG_ENDIAN
    hdr->length = 
      SWAP_LONG(hdr->length);
    hdr->chunk.fmt.format = 
      SWAP_WORD(hdr->chunk.fmt.format);
    hdr->chunk.fmt.nb_channel = 
      SWAP_WORD(hdr->chunk.fmt.nb_channel);
    hdr->chunk.fmt.sample_rate = 
      SWAP_LONG(hdr->chunk.fmt.sample_rate);
    hdr->chunk.fmt.byte_per_sec = 
      SWAP_LONG(hdr->chunk.fmt.byte_per_sec);
    hdr->chunk.fmt.byte_per_sample = 
      SWAP_WORD(hdr->chunk.fmt.byte_per_sample);
    hdr->chunk.fmt.bit_per_sample = 
      SWAP_WORD(hdr->chunk.fmt.bit_per_sample);
#endif /* BYTE_ORDER == BIG_ENDIAN */
    return 2;
  }

  if ( memcmp( hdr->type , "DATA" , 4 )==0 ||
       memcmp( hdr->type , "data" , 4 )==0) {
#if BYTE_ORDER == BIG_ENDIAN
    hdr->length = SWAP_LONG(hdr->length);
#endif /* BYTE_ORDER == BIG_ENDIAN */
    return 3;
  }

  return -1;
}

/*--------------------------------------------------------------------*/
/*! This function writes a header into a Wave File.
 */
int kb_wav_write_hdr( WAV * wav , COMMON_CHUNK_HEADER * hdr )
{
  unsigned int hdrlen=8;
  if ( memcmp( hdr->type , "RIFF" , 4 ) == 0 ||
       memcmp( hdr->type , "riff" , 4 ) == 0 ) {
#if BYTE_ORDER == BIG_ENDIAN
    hdr->length = SWAP_LONG(hdr->length);
#endif /* BYTE_ORDER == BIG_ENDIAN */
    hdrlen += sizeof(RIFF_CHUNK_HEADER);
  }
  else if ( memcmp( hdr->type , "FMT " , 4 ) == 0 ||
            memcmp( hdr->type , "fmt " , 4 ) == 0 ) {
#if BYTE_ORDER == BIG_ENDIAN
    hdr->length = 
      SWAP_LONG( hdr->length );
    
    hdr->chunk.fmt.format = 
      SWAP_WORD( hdr->chunk.fmt.format );
    
    hdr->chunk.fmt.nb_channel = 
      SWAP_WORD( hdr->chunk.fmt.nb_channel );
    
    hdr->chunk.fmt.sample_rate = 
      SWAP_LONG( hdr->chunk.fmt.sample_rate );
    
    hdr->chunk.fmt.byte_per_sec = 
      SWAP_LONG( hdr->chunk.fmt.byte_per_sec );
    
    hdr->chunk.fmt.byte_per_sample = 
      SWAP_WORD( hdr->chunk.fmt.byte_per_sample );
    
    hdr->chunk.fmt.bit_per_sample = 
      SWAP_WORD( hdr->chunk.fmt.bit_per_sample );
#endif /* BYTE_ORDER == BIG_ENDIAN */
    hdrlen += sizeof(FMT_CHUNK_HEADER);
  }
  else if ( memcmp( hdr->type , "DATA" , 4 ) == 0 ||
       memcmp( hdr->type , "data" , 4 ) == 0 ) {
#if BYTE_ORDER == BIG_ENDIAN
    hdr->length = SWAP_LONG( hdr->length );
#endif /* BYTE_ORDER == BIG_ENDIAN */
  }
  return fwrite( (void *)hdr , hdrlen , 1 , wav->file );
}



/*--------------------------------------------------------------------*/
/*! This function dumps the wav file header to the standard output.
 * \param wav	the wav struct
 */
void kb_wav_dump_format( WAV * wav )
{
  printf("%s\n" , (wav->fmt.format==1) ? "mono" : "stereo" );
  printf("%u channel(s)\n", wav->fmt.nb_channel);
  printf("%lu Hz sample rate \n" , wav->fmt.sample_rate);
  printf("%lu byte per second\n", wav->fmt.byte_per_sec);
  printf("%u byte per sample\n" , wav->fmt.byte_per_sample);
  printf("%u bit per sample\n", wav->fmt.bit_per_sample);
}
