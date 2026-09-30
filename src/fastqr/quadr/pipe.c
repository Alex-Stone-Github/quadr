#include "pipe.h"

// Allocate all requires resources
void QuadPipeline_init(size_t width, size_t height, struct QuadPipeline *pipe) {
    Substrate_init(width, height, &pipe->src);
    Substrate_init(width - 2, height - 2, &pipe->edges);
    Substrate_init(width - 4, height - 4, &pipe->skelx);
    Substrate_init(width - 4, height - 4, &pipe->skely);
    Substrate_init(width - 4, height - 4, &pipe->or);
    Contours_initFromSubstrate(&pipe->src, &pipe->contours);
    pipe->width = width;
    pipe->height = height;
}
// Process the image
void QuadPipeline_process(struct QuadPipeline *pipe,
                          struct BitmapInfo *bitmap_info) {
    // Copy bitmap into a substrate
    Substrate_updateBitmap(&pipe->src, bitmap_info);

    // Image Processing
    float edge_thresh = 0.1f;
    float erode_thresh = 0.1f;
    convolute(&pipe->src, &pipe->edges, &edge_detection_kernel);
    Substrate_stepPixels(&pipe->edges, edge_thresh);
    convolute(&pipe->edges, &pipe->skelx, &edge_erosion_kernel);
    Substrate_stepPixels(&pipe->skelx, erode_thresh);
    // could be a copy instead from erode
    Substrate_copyFrom(&pipe->skely, &pipe->skelx);
    Substrate_skeletonizeX(&pipe->skelx);
    Substrate_skeletonizeY(&pipe->skely);
    Substrate_or(&pipe->or, &pipe->skelx, &pipe->skely);

    // Find contours and squares
    Contours_findContoursConsumeSubstrate(&pipe->contours, &pipe->or);
    Squares_initFromContours(&pipe->contours, &pipe->squares);
}
// Cleanup resources
void QuadPipeline_deinit(struct QuadPipeline* pipe) {
    Substrate_deinit(&pipe->src);
    Substrate_deinit(&pipe->edges);
    Substrate_deinit(&pipe->skelx);
    Substrate_deinit(&pipe->skely);
    Substrate_deinit(&pipe->or);
    Contours_deinit(&pipe->contours);
}
