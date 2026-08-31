#include "contour.h"
#include "kernel.h"

#include <stdlib.h>
#include <assert.h>
#include <memory.h>

void Contours_initFromSubstrate(struct Substrate const *substrate,
                                struct Contours *contours) {
    // We can have at a maximum 1 point per pixel
    size_t pixel_count = substrate->width * substrate->height;
    contours->points_capacity = pixel_count;
    contours->points = calloc(pixel_count, sizeof(struct Contour));
    contours->points_length = 0;
    contours->contours_length = 0;
}
void Contours_findContoursInSubstrate(struct Contours *contours,
                                      struct Substrate const *substrate) {
    // Step 1: find all of the points in the image
    for (size_t x = 0; x < substrate->width; x ++) {
        for (size_t y = 0; y < substrate->height; y ++) {
          float pixel_value =
              *Substrate_getPixel((struct Substrate*)substrate, x, y);
          // Theororetically we can direct compare because direct assign to 1.0
          if (pixel_value == 1.0f) {
              //assert();
              struct Point2d new_point = {
                  (float)x, (float)y
              };
              assert(contours->points_length < MAX_CONTOURS);
              memcpy(&contours->points[contours->points_length++], &new_point,
                  sizeof(struct Point2d));
          }
        }
    }
}
void Contours_deinit(struct Contours *contours) {
    free(contours->points);
}
