#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

// Returns pixel at row x, col y, channel c
float get_pixel(image im, int x, int y, int c)
{
    // Clamp padding
    if (x < 0) {
        x = 0;
    } else if (x >= im.w) {
        x = im.w - 1;
    }

    if (y < 0) {
        y = 0;
    } else if (y >= im.h) {
        y = im.h - 1;
    }

    if (c < 0) {
        c = 0;
    } else if (c >= im.c) {
        c = im.c - 1;
    }

    return im.data[x + (y * im.w) + (c * im.w * im.h)];
}

// Set pixel at col x, row y, channel c
void set_pixel(image im, int x, int y, int c, float v)
{
    // Only do something for valid xyc
    if (x >= 0 && x < im.w &&
        y >= 0 && y < im.h &&
        c >= 0 && c < im.c) {
        im.data[x + (y * im.w) + (c * im.w * im.h)] = v;
    }
}

// Returns a new copy of im
image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    memcpy(copy.data, im.data, im.h * im.w * im.c * sizeof(float));
    return copy;
}

// Returns greyscale version of image
image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);

    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            float gray_pixel = 0.299 * get_pixel(im, col, row, 0) +
                               0.587 * get_pixel(im, col, row, 1) +
                               0.114 * get_pixel(im, col, row, 2);
            set_pixel(gray, col, row, 0, gray_pixel);
        }
    }
    return gray;
}

// Shift all pixels in channel c by +v
void shift_image(image im, int c, float v)
{
    for (int i = 0; i < im.h * im.w; i++) {
        im.data[c * im.h * im.w + i] += v;
    }
}

// Shift all pixels in channel c by *v
void scale_image(image im, int c, float v)
{
    for (int i = 0; i < im.h * im.w; i++) {
        im.data[c * im.h * im.w + i] *= v;
    }
}

void clamp_image(image im)
{
    for (int i = 0; i < im.h * im.w * im.c; i++) {
        if (im.data[i] < 0) {
            im.data[i] = 0;
        } 
        if (im.data[i] > 1) {
            im.data[i] = 1;
        }
    }
}

// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

// Convert im from RGB to HSV colorspace
void rgb_to_hsv(image im)
{
    // R -> H, G -> S, B -> V
    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            float r = get_pixel(im, col, row, 0);
            float g = get_pixel(im, col, row, 1);
            float b = get_pixel(im, col, row, 2);

            // Set V
            float v = three_way_max(r, g, b);
            set_pixel(im, col, row, 2, v);

            // Set S
            float s;
            float c;
            if (r == 0 && g == 0 && b == 0) {
                s = 0;
                c = 0;
            } else {
                c = v - three_way_min(r, g, b);
                s = c / v;
            }
            set_pixel(im, col, row, 1, s);

            // Set H
            float h_prime = 0;
            if (c == 0) {
                h_prime = 0;
            } else if (v == r) {
                h_prime = (g - b) / c;
            } else if (v == g) {
                h_prime = (b - r) / c + 2;
            } else if (v == b) {
                h_prime = (r - g) / c + 4;
            } 

            float h;
            if (h_prime < 0) {
                h = h_prime / 6 + 1;
            } else {
                h = h_prime / 6;
            }
            set_pixel(im, col, row, 0, h);
        }
    }
}

// Helper function for hsv_to_rgb
float hsv_to_rgb_function(int n, float h, float s, float v)
{
    float k = (float)fmod(n + (h / 60), 6);
    float temp = three_way_min(k, 4 - k, 1);
    if (temp < 0) {
        temp = 0;
    }
    return v - (v * s * temp);
}

// Convert im from HSV to RGB colorspace
void hsv_to_rgb(image im)
{
    // function
    // H -> R, S -> G, V -> B
    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            float h = get_pixel(im, col, row, 0);
            h *= 360; // Function works for h = [0, 360), so scale up
            float s = get_pixel(im, col, row, 1);
            float v = get_pixel(im, col, row, 2);

            // Convert to RGB using function
            float r = hsv_to_rgb_function(5, h, s, v);
            float g = hsv_to_rgb_function(3, h, s, v);
            float b = hsv_to_rgb_function(1, h, s, v);

            set_pixel(im, col, row, 0, r);
            set_pixel(im, col, row, 1, g);
            set_pixel(im, col, row, 2, b);
        }
    }
}