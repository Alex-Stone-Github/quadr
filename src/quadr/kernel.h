#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

/// A kernel is a 3 x 3 matrix used for convolutions of a substrate
#define KERNEL_SIZE 3
struct Kernel {
    /// Has columns of rows
    float window[KERNEL_SIZE * KERNEL_SIZE];
};
extern struct Kernel const edge_detection_kernel;
float Kernel_getItem(struct Kernel const* kernel, ssize_t offx, ssize_t offy);

/// A substrate is a normalized grayscale (width,height,1) image
/// Organized as columns of rows
struct Substrate {
    size_t width;
    size_t height;
    float* data;
};
/// This is a bitmap info struct which essentiailly represents a numpy array 
struct BitmapInfo {
    double* data;
    size_t nchannels;
    size_t npixels;
};
void Substrate_init(size_t width, size_t height, struct Substrate* substrate);
void Substrate_updateBitmap(struct Substrate *substrate,
                            struct BitmapInfo const* bitmap_info);
float* Substrate_getPixel(struct Substrate* substrate, size_t x, size_t y);
void Substrate_deinit(struct Substrate* substrate);

void convolute(struct Substrate const *src, struct Substrate *dst,
    struct Kernel const* kernel);

