#include "kernel.h"

#include <assert.h>
#include <stdbool.h>


struct Kernel const edge_detection_kernel = {{
    0.0, 1.0, 0.0,
    1.0, -4.0, 1.0,
    0.0, 1.0, 0.0,
    }};
float Kernel_getItem(struct Kernel const *kernel, ssize_t offx, ssize_t offy) {
    ssize_t xi = 1 + offx;
    ssize_t yi = 1 + offy;
    size_t index = KERNEL_SIZE * (size_t)yi + xi;
    assert(index < KERNEL_SIZE * KERNEL_SIZE);
    return kernel->window[index];
}
void Substrate_init(size_t width, size_t height, struct Substrate *substrate) {
    assert(width < 4000 && height < 4000); // provide a reasonalbe hieght
    substrate->width = width;
    substrate->height = height;
    substrate->data = calloc(width * height, sizeof(float));
    assert(substrate->data); // We ran out of memory
}
void Substrate_updateBitmap(struct Substrate *substrate,
                            struct BitmapInfo const *bitmap_info) {
    // We are going to convert a [0,255] bit map to a [0,1] and averaging out
    // channels if an rgb image.

    size_t npixels = substrate->height * substrate->width;
    assert(npixels == bitmap_info->npixels);
    assert(bitmap_info->nchannels == 3 || bitmap_info->nchannels == 1);
    bool is_rgb_image = bitmap_info->nchannels == 3;

    for (size_t i = 0; i < npixels; i++) {
        double max_pixel_val = 256.0;
        double gray = 0.0;
        if (is_rgb_image) {
            size_t nchannels = 3;
            double r = bitmap_info->data[i*nchannels] / max_pixel_val;
            double g = bitmap_info->data[i*nchannels + 1] / max_pixel_val;
            double b = bitmap_info->data[i*nchannels + 2] / max_pixel_val;
            gray = (r + g + b) / (double)nchannels;
        } else if (!is_rgb_image) {
            gray = bitmap_info->data[i] / max_pixel_val;
        }
        substrate->data[i] = gray;
    }
}
float* Substrate_getPixel(struct Substrate* substrate, size_t x, size_t y) {
    size_t index = substrate->width * y + x;
    assert(index < substrate->width * substrate->height);
    return &substrate->data[index];
}
void Substrate_deinit(struct Substrate *substrate) {
    free(substrate->data);
}

static float convoluteSample(struct Substrate const *src,
                            struct Kernel const *kernel, size_t x, size_t y) {
    // Ensure we are dealing with valid data
    assert(src && src->data);
    assert(kernel);
    float summation = 0;
    for (ssize_t xoff = -1; xoff <= 1; xoff++) {
        for (ssize_t yoff = -1; yoff <= 1; yoff++) {
            ssize_t _x = x;
            ssize_t _y = y;
            float kernel_value = Kernel_getItem(kernel, xoff, yoff);
            float src_value = *Substrate_getPixel((struct Substrate *)src,
                _x + xoff, _y + yoff);
            summation += kernel_value * src_value;
        }
    }
    return summation;
}
void convolute(struct Substrate const *src, struct Substrate *dst,
               struct Kernel const *kernel) {
    // Ensure we are dealing with valid data
    assert(src && src->data);
    assert(dst && dst->data);
    assert(kernel);
    assert(dst->width == src->width - 2);
    assert(dst->height == src->height - 2);

    for (size_t x = 1; x < dst->width - 1; x++) {
        for (size_t y = 1; y < dst->height - 1; y++) {
            float new_value = convoluteSample(src, kernel, x, y);
            *Substrate_getPixel(dst, x - 1, y - 1) = new_value;
        }
    }
}
