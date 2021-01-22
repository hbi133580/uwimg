#include <math.h>
#include "image.h"

float nn_interpolate(image im, float x, float y, int c)
{
    return get_pixel(im, roundf(x), roundf(y), c);
}

image nn_resize(image im, int w, int h)
{
    image resized = make_image(w, h, im.c);

    // For each pixel in resized image, fill using info from old image
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            for (int channel = 0; channel < im.c; channel++) {
                float resized_row = (row + 0.5) * (im.h / (float)h) - 0.5;
                float resized_col = (col + 0.5) * (im.w / (float)w) - 0.5;
                float resized_color = nn_interpolate(im, resized_col, resized_row, channel);
                set_pixel(resized, col, row, channel, resized_color);
            }
        }
    }
    return resized;
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    // q1 = V1*d2 + V2*d1
    // q2 = V3*d2 + V4*d1
    // q = q1*d4 + q2*d3

    float v1 = get_pixel(im, floorf(x), floorf(y), c);
    float v2 = get_pixel(im, ceilf(x), floorf(y), c);
    float v3 = get_pixel(im, floorf(x), ceilf(y), c);
    float v4 = get_pixel(im, ceilf(x), ceilf(y), c);
    float q1 = (v1 * (ceilf(x) - x)) + (v2 * (x - floorf(x)));
    float q2 = (v3 * (ceilf(x) - x)) + (v4 * (x - floorf(x)));
    return (q1 * (ceilf(y) - y)) + (q2 * (y - floorf(y)));;
}

image bilinear_resize(image im, int w, int h)
{
    image resized = make_image(w, h, im.c);

    // For each pixel in resized image, fill using info from old image
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            for (int channel = 0; channel < im.c; channel++) {
                float resized_row = (row + 0.5) * (im.h / (float)h) - 0.5;
                float resized_col = (col + 0.5) * (im.w / (float)w) - 0.5;
                float resized_color = bilinear_interpolate(im, resized_col, resized_row, channel);
                set_pixel(resized, col, row, channel, resized_color);
            }
        }
    }
    return resized;
}

