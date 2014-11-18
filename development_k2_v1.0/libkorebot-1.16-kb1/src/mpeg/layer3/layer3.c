/* layer3.c */
 
#include "../mpegtypes.h"
//#include "wave.h"
#include "layer3.h"
#include "l3subband.h"
#include "l3mdct.h"
#include "l3loop.h"
#include "bitstream.h"
#include "l3bitstrea.h"

/*
 * update_status:
 * --------------
 */
static void update_status(struct config_t *cfg, int frames_processed)
{
  printf("\015[Frame %6d of %6ld] (%2.2f%%)", 
            frames_processed,cfg->mpeg.total_frames,
            (double)((double)frames_processed/cfg->mpeg.total_frames)*100); 
  fflush(stdout);
}

/*
 * L3_compress:
 * ------------
 */
void L3_compress(struct config_t *cfg)
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

  open_bit_stream_w(&bs, cfg->outfile, BUFFER_SIZE);
  
  memset((char *)&side_info,0,sizeof(L3_side_info_t));

  L3_subband_initialise();
  L3_mdct_initialise();
  L3_loop_initialise();
  
  cfg->mpeg.samples_per_frame = samp_per_frame;
  cfg->mpeg.total_frames      = cfg->wave.total_samples/cfg->mpeg.samples_per_frame;
  cfg->mpeg.bits_per_slot     = 8;
  frames_processed              = 0;
  sideinfo_len = (cfg->wave.channels==1) ? 168 : 288;

  /* Figure average number of 'slots' per frame. */
  avg_slots_per_frame   = ((double)cfg->mpeg.samples_per_frame / 
                           ((double)cfg->wave.samplerate/1000)) *
                          ((double)cfg->mpeg.bitr /
                           (double)cfg->mpeg.bits_per_slot);
  whole_slots_per_frame = (int)avg_slots_per_frame;
  frac_slots_per_frame  = avg_slots_per_frame - (double)whole_slots_per_frame;
  slot_lag              = -frac_slots_per_frame;
  if(frac_slots_per_frame==0)
    cfg->mpeg.padding = 0;

  while(wave_get(cfg, buffer))
  {
    update_status(cfg, ++frames_processed);

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
      for(channel=cfg->wave.channels; channel--; )
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


