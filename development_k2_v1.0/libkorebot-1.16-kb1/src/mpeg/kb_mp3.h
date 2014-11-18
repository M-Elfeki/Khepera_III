/*--------------------------------------------------------------------*/

#ifndef __mp3_h__
#define __mp3_h__



struct mpeg_t {
    FILE *file;
    int  type;
    int  layr;
    int  mode;
    int  bitr;
    int  psyc;
    int  emph;
    int  padding;
    long samples_per_frame;
    long bits_per_frame;
    long bits_per_slot;
    long total_frames;
    int  bitrate_index;
    int  samplerate_index;
    int crc;
    int ext;
    int mode_ext;
    int copyright;
    int original;
};

struct wave_t {
    FILE *file;
    int  type;
    int  channels;
    int  bits;
    long samplerate;
    long total_samples;
    long length;
};

struct config_t {
    time_t start_time;

    char* infile;
    struct wave_t wave;

    char *outfile;
    
    struct mpeg_t mpeg;
};


/*--------------------------------------------------------------------*/

extern void kb_mp3_updStatus(struct config_t *cfg, int frames_processed);

extern int kb_mp3_getSmplIndex(unsigned int freq);

extern int kb_mp3_getBitrIndex(unsigned int bitr);

extern void kb_mp3_setDefault( struct config_t *cfg );

extern int kb_mp3_Compress( unsigned short *smplFrame, unsigned long smplNmbr, struct config_t *cfg, int bitRate,
                     char *filePath );
		     
extern void kb_mp3_chkCfg(struct config_t *cfg, int smplRate); 

/*--------------------------------------------------------------------*/

#endif /* __mp3_h__ */
