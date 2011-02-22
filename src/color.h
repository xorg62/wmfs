#ifndef COLOR_H
#define COLOR_H

#include "structs.h"

double color_clamp(double, double, double);
uint   color_pack_rgb(uint, uint, uint);
uint   color_shade(uint, double);
void   color_unpack_rgb(uint, uint*, uint*, uint*);
void   color_rgb_to_hsl(uint, uint, uint, double*, double*, double*);
void   color_hsl_to_rgb(double, double, double, uint*, uint*, uint*);

#endif
