#ifndef RESERVOIR_H
#define RESERVOIR_H

void ResvFrameBegin(L3_side_info_t *l3_side, int mean_bits, int frameLength );
int ResvMaxBits (struct config_t *cfg, L3_side_info_t *l3_side, double *pe, int mean_bits );
void ResvAdjust    (struct config_t *cfg, gr_info *gi, L3_side_info_t *l3_side, int mean_bits );
void ResvFrameEnd  (struct config_t *cfg, L3_side_info_t *l3_side, int mean_bits );

#endif
