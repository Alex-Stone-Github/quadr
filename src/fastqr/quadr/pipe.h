#pragma once

#include "contour.h"
#include "kernel.h"
#include "square.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// This pipline keeps track of memory resources for processing an image
struct QuadPipeline {
    struct Substrate src, edges, skelx, skely, or;
    struct Contours contours;
    struct Squares squares;
    size_t width, height;
};
void QuadPipeline_init(size_t width, size_t height, struct QuadPipeline* pipe);
void QuadPipeline_process(struct QuadPipeline *pipe,
                          struct BitmapInfo* bitmap_info);
void QuadPipeline_deinit(struct QuadPipeline* pipe);
