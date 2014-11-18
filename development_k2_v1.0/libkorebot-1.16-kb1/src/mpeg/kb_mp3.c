/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * $Author: pbureau $
 * $Date: 2006/04/08 15:54:36 $
 * $Revision: 1.2 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/mpeg/kb_mp3.c,v 1.2 2006/04/08 15:54:36 pbureau Exp $
 */

/*--------------------------------------------------------------------*/
/*! 
 * \file   kb_mp3.c mp3 files handling
 *
 * \brief
 *         This module provides functions write some samples as a mp3 
 *         mp3 file.
 *         todo: add mpeg layer 3 decoding to support mp3 read as well
 *
 * \author   Arnaud Maye (K-Team SA)
 * \note     Copyright (C) 2005 K-TEAM SA
 */

#include "../korebot.h"
#include "mpegtypes.h"
#include "layer3/layer3.h"
#include "layer3/l3subband.h"
#include "layer3/l3mdct.h"
#include "layer3/l3loop.h"
#include "layer3/bitstream.h"
#include "layer3/l3bitstrea.h"

#define print_type(t)  printf("%c%c%c%c\n" , t[0], t[1], t[2], t[3])

/*--------------------------------------------------------------------*/
/*! Check that a given sample rate frequency is valid and return its
 * index it might be 44100, 48000 or 32000.
 *
 * \param freq	frequency 
 * \return smpl rate index :
 *  - 0   => 44100 Hz
 *  - 1   => 48000 Hz
 *  - 2   => 32000 Hz
 *  - < 0 => Invalid
 */
int kb_mp3_getSmplIndex(unsigned int freq)
{
  unsigned int mpeg1[3] = {44100, 48000, 32000};
  int i;

  for(i=0;i<3;i++)
    if(freq==mpeg1[i]) return i;
    
  return -1;
  
}

/*--------------------------------------------------------------------*/
/*! Check that a given bit rate is valid and return its bit rate index
 * it might be 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224,
 * 256,320
 *
 * \param freq	frequency 
 * \return smpl rate index :
 *  - 0   => 0 Hz
 *  - 1   => 32 Hz
 *  - 2   => 40 Hz
 *  - 3   => 48 Hz
 *  - 4   => 56 Hz
 *  - 5   => 64 Hz
 *  - 6   => 80 Hz
 *  - 7   => 96 Hz
 *  - 8   => 112 Hz
 *  - 9   => 128 Hz
 *  - 10  => 160 Hz
 *  - 11  => 192 Hz
 *  - 12  => 224 Hz
 *  - 13  => 256 Hz
 *  - 14  => 320 Hz
 *  - < 0 => Invalid
 */
int kb_mp3_getBitrIndex(unsigned int bitr)
{
  unsigned int mpeg1[15] = {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320};
  int i;

  for(i=0;i<15;i++)
    if(bitr==mpeg1[i]) return i;

  return -1;
}

/*--------------------------------------------------------------------*/
/*! This function set the default mpeg layer 3 compression settings
 *
 * \param cfg	pointer to an initialized config_t structure 
 */
void kb_mp3_setDefault( struct config_t *cfg )
{
  cfg->mpeg.type = 1;
  cfg->mpeg.layr = 2;
  cfg->mpeg.mode = 2;
  cfg->mpeg.bitr = 128;
  cfg->mpeg.psyc = 2;
  cfg->mpeg.emph = 0; 
  cfg->mpeg.crc  = 0;
  cfg->mpeg.ext  = 0;
  cfg->mpeg.mode_ext  = 0;
  cfg->mpeg.copyright = 0;
  cfg->mpeg.original  = 1;
}



/*
 * update_status:
 * --------------
 */
void kb_mp3_updStatus(struct config_t *cfg, int frames_processed)
{
  printf("\015[Frame %6d of %6ld] (%2.2f%%)", 
            frames_processed,cfg->mpeg.total_frames,
            (double)((double)frames_processed/cfg->mpeg.total_frames)*100); 
  fflush(stdout);
}


void kb_mp3_chkCfg(struct config_t *cfg, int smplRate)
{
  static char *mode_names[4]    = { "stereo", "j-stereo", "dual-ch", "mono" };
  static char *layer_names[3]   = { "I", "II", "III" };
  static char *version_names[2] = { "MPEG-II (LSF)", "MPEG-I" };
  static char *psy_names[3]     = { "", "MUSICAM", "Shine" };
  static char *demp_names[4]    = { "none", "50/15us", "", "CITT" };

  cfg->mpeg.samplerate_index = kb_mp3_getSmplIndex(smplRate);
  cfg->mpeg.bitrate_index    = kb_mp3_getBitrIndex(cfg->mpeg.bitr);

  printf("%s layer %s, %s  Psychoacoustic Model: %s\n",
           version_names[cfg->mpeg.type],
           layer_names[cfg->mpeg.layr], 
           mode_names[cfg->mpeg.mode],
           psy_names[cfg->mpeg.psyc]);
  printf("Bitrate=%d kbps  ",cfg->mpeg.bitr );
  printf("De-emphasis: %s   %s %s\n",
          demp_names[cfg->mpeg.emph], 
          ((cfg->mpeg.original)?"Original":""),
          ((cfg->mpeg.copyright)?"(C)":""));
}


/*--------------------------------------------------------------------*/
/*! Compress a given smplFrame at a given bitRate 
 *
 * \param smplFrame	a buffer containing samples to be compressed to layer3
 * \param cfg	pointer to an initialized config_t structure
 * \param bitRate	bitRate to encode sample at 
 * \return status :
 *  - 0   => Success
 *  < 0   => any error code
 */
int kb_mp3_Compress( unsigned short *smplFrame, unsigned long smplNmbr, struct config_t *cfg, int bitRate,
                     char *filePath )
{
  int             frames_processed;
  int             channel;
  int             i;
  int             gr;
  double          pe[2][2];
  short          *buffer_window[2];
  double          avg_slots_per_frame;
  double          frac_slots_per_frame;
  long            whole_slots_per_frame;
  double          slot_lag;
  int             mean_bits;
  int             sideinfo_len;
  static short    buffer[2][samp_per_frame];
  static int      l3_enc[2][2][samp_per_frame2];
  static long     l3_sb_sample[2][3][18][SBLIMIT];
  static long     mdct_freq[2][2][samp_per_frame2];
  static L3_psy_ratio_t  ratio;
  static L3_side_info_t  side_info;
  static L3_scalefac_t   scalefactor;
  static bitstream_t     bs;
  
  
  if(cfg)
    cfg->mpeg.bitr =  bitRate; /* Override the default bitRate */
    
    
  if(filePath && cfg)
    cfg->outfile = filePath;
  else
    return -1; /* outfile related error */
    
  open_bit_stream_w(&bs, cfg->outfile, BUFFER_SIZE);
  
  memset((char *)&side_info,0,sizeof(L3_side_info_t));
  
  /* some mpeg layer 3 initialisations */
  L3_subband_initialise();
  L3_mdct_initialise();
  L3_loop_initialise();
  
  cfg->mpeg.samples_per_frame = samp_per_frame;
  cfg->mpeg.total_frames      = smplNmbr /cfg->mpeg.samples_per_frame;
  cfg->mpeg.bits_per_slot     = 8;
  frames_processed            = 0;
  
  sideinfo_len = 288; /* Stereo assumed */ /* (config.wave.channels==1) ? 168 : 288; */
  
   /* Figure average number of 'slots' per frame. */
  avg_slots_per_frame   = ((double)cfg->mpeg.samples_per_frame / 
                           ((double)smplNmbr/1000)) *
                          ((double)cfg->mpeg.bitr /
                           (double)cfg->mpeg.bits_per_slot);
  whole_slots_per_frame = (int)avg_slots_per_frame;
  frac_slots_per_frame  = avg_slots_per_frame - (double)whole_slots_per_frame;
  slot_lag              = -frac_slots_per_frame;
  if(frac_slots_per_frame==0)
    cfg->mpeg.padding = 0;
    
  //while(wave_get(buffer))
  for( i = 0; i < smplNmbr; i++)
  {
    kb_mp3_updStatus(cfg, ++frames_processed);

    buffer_window[0] = buffer[0];
    buffer_window[1] = buffer[1];

    if(frac_slots_per_frame)
      if(slot_lag>(frac_slots_per_frame-1.0))
      { /* No padding for this frame */
        slot_lag    -= frac_slots_per_frame;
        cfg->mpeg.padding = 0;
      }
      else 
      { /* Padding for this frame  */
        slot_lag    += (1-frac_slots_per_frame);
        cfg->mpeg.padding = 1;
      }
      
    cfg->mpeg.bits_per_frame = 8*(whole_slots_per_frame + cfg->mpeg.padding);
    mean_bits = (cfg->mpeg.bits_per_frame - sideinfo_len)>>1;

    /* polyphase filtering */
    for(gr=0;gr<2;gr++)
      for(channel=2; channel-- ; ) /* assumed stereo */ /* for(channel=cfg->wave.channels; channel--; ) */
        for(i=0;i<18;i++)
          L3_window_filter_subband(&buffer_window[channel], &l3_sb_sample[channel][gr+1][i][0] ,channel);

    /* apply mdct to the polyphase output */
    L3_mdct_sub(cfg, l3_sb_sample, mdct_freq);

    /* bit and noise allocation */
    L3_iteration_loop(cfg, pe,mdct_freq,&ratio,&side_info, l3_enc, mean_bits,&scalefactor);

    /* write the frame to the bitstream */
    L3_format_bitstream(cfg, l3_enc,&side_info,&scalefactor, &bs,mdct_freq,NULL,0);

  }    
  close_bit_stream(&bs);  
  
}

