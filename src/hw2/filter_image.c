#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

void l1_normalize(image im)
{
    float sum = 0;
    for (int i = 0; i < im.h * im.w * im.c; i++) {
        sum += im.data[i];
    }
    for (int i = 0; i < im.h * im.w * im.c; i++) {
        im.data[i] /= sum;
    }
}

image make_box_filter(int w)
{
    image filter = make_image(w, w, 1);
    for (int i = 0; i < w * w; i++) {
        filter.data[i] = 1.0 / (w * w);
    }
    return filter;
}

image convolve_image(image im, image filter, int preserve)
{
    assert (filter.c == 1 || filter.c == im.c);
    image result = make_image(im.w, im.h, preserve ? im.c : 1);
    int padding = filter.w / 2;
    
    for (int channel = 0; channel < im.c; channel++) {
        for (int row = 0; row < im.h; row++) {
            for (int col = 0; col < im.w; col++) {
                float weighted_sum = preserve ? 0 : get_pixel(result, col, row, 0);;

                // Find convoluted pixel value
                for (int f_row = 0; f_row < filter.h; f_row++) {
                    for (int f_col = 0; f_col < filter.w; f_col++) {
                        weighted_sum += 
                                get_pixel(im, col + f_col - padding, row + f_row - padding, channel)
                                * get_pixel(filter, f_col, f_row, filter.c == 1 ? 0 : channel);
                    } 
                }
                set_pixel(result, col, row, preserve ? channel : 0, weighted_sum);
            }
        }
    }
    return result;
}

image make_highpass_filter()
{
    image filter = make_image(3, 3, 1);
    filter.data[0] = 0;
    filter.data[1] = -1;
    filter.data[2] = 0;
    filter.data[3] = -1;
    filter.data[4] = 4;
    filter.data[5] = -1;
    filter.data[6] = 0;
    filter.data[7] = -1;
    filter.data[8] = 0;
    return filter;
}

image make_sharpen_filter()
{
    image filter = make_image(3, 3, 1);
    filter.data[0] = 0;
    filter.data[1] = -1;
    filter.data[2] = 0;
    filter.data[3] = -1;
    filter.data[4] = 5;
    filter.data[5] = -1;
    filter.data[6] = 0;
    filter.data[7] = -1;
    filter.data[8] = 0;
    return filter;
}

image make_emboss_filter()
{
    image filter = make_image(3, 3, 1);
    filter.data[0] = -2;
    filter.data[1] = -1;
    filter.data[2] = 0;
    filter.data[3] = -1;
    filter.data[4] = 1;
    filter.data[5] = 1;
    filter.data[6] = 0;
    filter.data[7] = 1;
    filter.data[8] = 2;
    return filter;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// 
// Answer: Preserve preserves the three separate channels for color (RBG), so we should use
// preserve for filters that output color images. This means we should use preserve for all 
// the above filters except for highpass.

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
//
// Answer: Yes, for all the above filters except the box filter, it's possible that a pixel in 
// the resulting image is out of range, so we need to process the image to keep all pixel
// values between 0 and 1.

image make_gaussian_filter(float sigma)
{
    int size = (int)ceilf(6 * sigma);
    if (size % 2 == 0) { 
        size++;
    }
    image filter = make_image(size, size, 1);
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            int x_p = x - size / 2;
            int y_p = y - size / 2;
            float gaussian_value = (1 / (2 * M_PI * sigma * sigma)) * 
                                   expf(-(x_p*x_p + y_p*y_p) / (2 * sigma * sigma));
            set_pixel(filter, x, y, 0, gaussian_value);
        }
    }
    l1_normalize(filter);
    return filter;
}

image add_image(image a, image b)
{
    assert (a.h == b.h && a.w == b.w && a.c == b.c);
    image result = make_image(a.w, a.h, a.c);

    for (int i = 0; i < a.w * a.h * a.c; i++) {
        result.data[i] = a.data[i] + b.data[i];
    }
    return result;
}

image sub_image(image a, image b)
{
    assert (a.h == b.h && a.w == b.w && a.c == b.c);
    image result = make_image(a.w, a.h, a.c);

    for (int i = 0; i < a.w * a.h * a.c; i++) {
        result.data[i] = a.data[i] - b.data[i];
    }
    return result;
}

image make_gx_filter()
{
    image filter = make_image(3, 3, 1);
    filter.data[0] = -1;
    filter.data[1] = 0;
    filter.data[2] = 1;
    filter.data[3] = -2;
    filter.data[4] = 0;
    filter.data[5] = 2;
    filter.data[6] = -1;
    filter.data[7] = 0;
    filter.data[8] = 1;
    return filter;
}

image make_gy_filter()
{
    image filter = make_image(3, 3, 1);
    filter.data[0] = -1;
    filter.data[1] = -2;
    filter.data[2] = -1;
    filter.data[3] = 0;
    filter.data[4] = 0;
    filter.data[5] = 0;
    filter.data[6] = 1;
    filter.data[7] = 2;
    filter.data[8] = 1;
    return filter;
}

void feature_normalize(image im)
{
    float max = 0;
    float min = 1;
    for (int i = 0; i < im.h * im.w * im.c; i++) {
        if (im.data[i] > max) {
            max = im.data[i];
        }
        if (im.data[i] < min) {
            min = im.data[i];
        }
    }

    float range = max - min;
    if (range == 0) {
        for (int i = 0; i < im.h * im.w * im.c; i++) {
            im.data[i] = 0;
        }
    } else {
        for (int i = 0; i < im.h * im.w * im.c; i++) {
            im.data[i] -= min;
            im.data[i] /= range;
        }
    }
}

image *sobel_image(image im)
{   
    image *result = calloc(2, sizeof(image));

    image gx_filter = make_gx_filter();
    image gy_filter = make_gy_filter();
    image gx = convolve_image(im, gx_filter, 0);
    image gy = convolve_image(im, gy_filter, 0);
    
    image g = make_image(im.w, im.h, 1);
    image theta = make_image(im.w, im.h, 1);

    for (int i = 0; i < im.w * im.h; i++) {
        g.data[i] = sqrtf(gx.data[i]*gx.data[i] + gy.data[i]*gy.data[i]);
        theta.data[i] = atan2f(gy.data[i], gx.data[i]);
    }
    result[0] = g;
    result[1] = theta;

    free_image(gx_filter);
    free_image(gy_filter);
    free_image(gx);
    free_image(gy);
    return result;
}

image colorize_sobel(image im)
{
    image result = make_image(im.w, im.h, 3);
    image *res = sobel_image(im);
    feature_normalize(res[0]);
    feature_normalize(res[1]);

    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            set_pixel(result, col, row, 0, get_pixel(res[1], col, row, 0));
            set_pixel(result, col, row, 1, get_pixel(res[0], col, row, 0));
            set_pixel(result, col, row, 2, get_pixel(res[0], col, row, 0));
        }
    }
    return result;
}
