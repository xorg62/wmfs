/*
*      color.c
*      Copyright © 2011 Brian Mock <mock.brian@gmail.com>
*      All rights reserved.
*
*      Redistribution and use in source and binary forms, with or without
*      modification, are permitted provided that the following conditions are
*      met:
*
*      * Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*      * Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following disclaimer
*        in the documentation and/or other materials provided with the
*        distribution.
*      * Neither the name of the  nor the names of its
*        contributors may be used to endorse or promote products derived from
*        this software without specific prior written permission.
*
*      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "wmfs.h"

/** Clamp a number x within the range [a, b].
 * \param x the number which to clamp
 * \param a the lowest possible value
 * \param b the highest possible value
 * \return the clamped number
 */
static double
color_clamp(double x, double a, double b)
{
    if(x < a)
        return a;
    else if(x > b)
        return b;
    else
        return x;
}

/** Pack a triplet of RGB values into a single uint
 * \param r the red value
 * \param g the green value
 * \param b the blue value
 * \return the packed RGB value
 */
static uint
color_pack_rgb(uint r, uint g, uint b)
{
    return (r << 16) | (g << 8) | b;
}

/** Unpack an RGB uint into three separate values
 * \param rgb the packed color
 * \param r a pointer to a uint where the red value will be stored
 * \param g a pointer to a uint where the green value will be stored
 * \param b a pointer to a uint where the blue value will be stored
 */
static void
color_unpack_rgb(uint rgb, uint *r, uint *g, uint *b)
{
    *r = (rgb >> 16) & 0xFF;
    *g = (rgb >>  8) & 0xFF;
    *b =  rgb        & 0xFF;
}

/** Convert unpacked RGB values into HSL, storing in the doubles referenced
 * by the pointers h, s, l
 */
static void
color_rgb_to_hsl(uint xr, uint xg, uint xb, double *h, double *s, double *l)
{
    double r = xr/255.0;
    double g = xg/255.0;
    double b = xb/255.0;

    double v;
    double m;
    double vm;
    double r2, g2, b2;

    *h = 0;
    *s = 0;
    *l = 0;

    /* v is max(r, g, b)
     * m is min(r, g, b)
     */
    v = r > g ? r : g;
    v = v > b ? v : b;
    m = r < g ? r : g;
    m = m < b ? m : b;

    *l = (m + v)/2.0;

    if(*l <= 0.0)
        return;

    vm = v - m;
    *s = vm;

    if(*s > 0.0)
        *s /= (*l <= 0.5) ? (v + m) : (2.0 - v - m);
    else
        return;

    r2 = (v - r)/vm;
    g2 = (v - g)/vm;
    b2 = (v - b)/vm;

    if(r == v)
        *h = (g == m ? 5.0 + b2 : 1.0 - g2);
    else if(g == v)
        *h = (b == m ? 1.0 + r2 : 3.0 - b2);
    else
        *h = (r == m ? 3.0 + g2 : 5.0 - r2);

    *h /= 6.0;
}

/** Convert h, s, l values to RGB and store them in the three uint
 * referenced by the last three parameters.
 */
static void
color_hsl_to_rgb(double h, double sl, double l, uint *rx, uint *gx, uint *bx)
{
    double v;
    double r,g,b;

    r = l;
    g = l;
    b = l;
    v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
    if(v > 0)
    {
        double m;
        double sv;
        int    sextant;
        double fract, vsf, mid1, mid2;

        m = l + l - v;
        sv = (v - m ) / v;
        h *= 6.0;
        sextant = (int) h;
        fract = h - sextant;
        vsf = v * sv * fract;
        mid1 = m + vsf;
        mid2 = v - vsf;
        switch(sextant)
        {
            case 0:
                r = v;
                g = mid1;
                b = m;
                break;
            case 1:
                r = mid2;
                g = v;
                b = m;
                break;
            case 2:
                r = m;
                g = v;
                b = mid1;
                break;
            case 3:
                r = m;
                g = mid2;
                b = v;
                break;
            case 4:
                r = mid1;
                g = m;
                b = v;
                break;
            case 5:
                r = v;
                g = m;
                b = mid2;
                break;
        }
    }

    *rx = r * 255.0;
    *gx = g * 255.0;
    *bx = b * 255.0;
}

/** Shades a color by the amount. This works by converting a packed RGB
 * color to HSL, adding the amount to the lightness,
 * and then converting back to RGB. 1.0 is max lightness, 0.0 is min lightness.
 * \param shadeVal the amount to shade the lightness by.
 * \return the shaded color
 */
uint
color_shade(uint rgb, double shadeVal)
{
    uint   r, g, b;
    double h, s, l;

    color_unpack_rgb(rgb, &r, &g, &b);
    color_rgb_to_hsl(r, g, b, &h, &s, &l);

    l += shadeVal;

    l = color_clamp(l, 0, 1);

    color_hsl_to_rgb(h, s, l, &r, &g, &b);
    rgb = color_pack_rgb(r, g, b);

    return rgb;
}
/* vim: et ts=5 sts=5 sw=5:
 */
