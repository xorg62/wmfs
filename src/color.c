// author: Brian Mock <mock.brian@gmail.com>

#include "color.h"

uint
color_shade(uint rgb, double shadeVal)
{
    uint   r, g, b;
    double h, s, l;

    unpack_rgb(rgb, &r, &g, &b);
    rgb_to_hsl(r, g, b, &h, &s, &l);

    l += shadeVal;

    l = clamp(l, 0, 1);

    hsl_to_rgb(h, s, l, &r, &g, &b);
    rgb = pack_rgb(r, g, b);

    return rgb;
}

double
color_clamp(double x, double a, double b)
{
    if(x < a)
        return a;
    else if(x > b)
        return b;
    else
        return x;
}

uint
color_pack_rgb(uint r, uint g, uint b)
{
    return (r << 16) | (g << 8) | b;
}

void
color_unpack_rgb(uint rgb, uint *r, uint *g, uint *b)
{
    *r = (rgb >> 16) & 0xFF;
    *g = (rgb >>  8) & 0xFF;
    *b =  rgb        & 0xFF;
}

void
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

    v = MAX(r, g);
    v = MAX(v, b);
    m = MIN(r, g);
    m = MIN(m, b);

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

void
color_hsl_to_rgb(double h, double sl, double l, uint *rx, uint *gx, uint *bx)
{
    double v;
    double r,g,b;

    r = l;
    g = l;
    b = l;
    v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
    if (v > 0) {
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
