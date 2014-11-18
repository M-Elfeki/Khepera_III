/* 
 * wav.h
 */

#ifndef __WAV_H__
#define __WAV_H__

#define SWAP_WORD(w)  (((w>>8)&0xff)|((w<<8)&0xff00))
#define SWAP_LONG(l)  (((l>>24)&0xff)|((l>>8)&0xff00)|\
                       ((l<<8)&0xff0000)|((l<<24)&0xff000000))

#define WAV_State_Hdr   0
#define WAV_State_Data  1

typedef struct
{
  unsigned char chunk_content[4];
}
RIFF_CHUNK_HEADER;

typedef struct 
{
  unsigned short format;
  unsigned short nb_channel;
  unsigned long  sample_rate;
  unsigned long  byte_per_sec;
  unsigned short byte_per_sample;
  unsigned short bit_per_sample;
}
FMT_CHUNK_HEADER;

typedef struct
{
  unsigned char type[4];
  unsigned long length;
  union {
    RIFF_CHUNK_HEADER riff;
    FMT_CHUNK_HEADER  fmt;
  } 
  chunk;
}
COMMON_CHUNK_HEADER;

typedef struct
{
    FILE *file;
    FMT_CHUNK_HEADER  fmt;
    unsigned int length;
    int modified;
}
WAV;

/*--------------------------------------------------------------------*/

extern void kb_wav_init( WAV * wav );

extern int  kb_wav_open( WAV * wav , const char * file , int read );

extern void kb_wav_close( WAV * wav );

extern int  kb_wav_read_data( WAV * wav , 
                           void * buf , unsigned int size );

extern int  kb_wav_write_data( WAV * wav ,
                            void * buf , unsigned int len );

extern void kb_wav_set_format( WAV * wav ,
                            unsigned short format ,
                            unsigned short nb_channel ,
                            unsigned long  sample_rate ,
                            unsigned short bit_per_sample );

extern int kb_wav_read_hdr( WAV * wav , COMMON_CHUNK_HEADER * hdr );

extern int kb_wav_write_hdr( WAV * wav , COMMON_CHUNK_HEADER * hdr );

extern void kb_wav_dump_format( WAV * wav );

#endif /* __WAV_H__ */
