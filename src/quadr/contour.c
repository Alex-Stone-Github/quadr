#include "contour.h"
#include "kernel.h"

#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

#define BRIDGE_DIST 15.0f

void Contours_initFromSubstrate(struct Substrate const *substrate,
                                struct Contours *contours) {
    // We can have at a maximum 1 point per pixel
    size_t pixel_count = substrate->width * substrate->height;
    contours->points_capacity = pixel_count;
    contours->points = calloc(pixel_count, sizeof(struct Contour));
    contours->points_length = 0;
    contours->contours_length = 0;
}
static bool findFreePixel(struct Substrate const *substrate,
                          size_t* xid, size_t* yid) {
    // Returns null if no point is found
    for (size_t x = 0; x < substrate->width; x ++) {
        for (size_t y = 0; y < substrate->height; y ++) {
            float pixel_value =
                *Substrate_getPixel((struct Substrate*)substrate, x, y);
            // Theororetically we can direct compare directly because of assign
            if (pixel_value == 1.0f) {
                *xid = x;
                *yid = y;
                return true;
            }
        }
    }
    return false;
}
static size_t clampSize(ssize_t x, size_t width) {
    assert(width > 0 && width < 100000);
    // Make in range
    while (x < 0) x += width;
    while (x >= (ssize_t)width) x -= width;
    return x;
}
float distanceSqFlt(float ax, float ay, float bx, float by) {
    return powf(ax - bx, 2.0f) + powf(ay - by, 2.0f);
}
static float distanceSq(size_t ax, size_t ay, size_t bx, size_t by) {
    return distanceSqFlt((float)ax, (float)ay, (float)bx, (float)by);
}
void Contours_findContoursConsumeSubstrate(struct Contours *contours,
                                      struct Substrate const *substrate) {
    // While we have a pixel available
    size_t current_xid, current_yid;
    while (findFreePixel(substrate, &current_xid, &current_yid)) {
        // Chain points together to form a contour
        size_t contour_start = contours->points_length;
        while (true) {
            // Add the current point to the current contour
            struct Point2d current_point = {
            (float)current_xid, (float)current_yid,
            };
            assert(contours->contours_length < MAX_CONTOURS);
            memcpy(&contours->points[contours->points_length++],
                   &current_point, sizeof(struct Point2d));
            *Substrate_getPixel((struct Substrate *)substrate,
                current_xid, current_yid) = 0.0f;

            // Find a point to bridge to
            float smallest_dist_sq = FLT_MAX;
            size_t best_bridgeto_xid, best_bridgeto_yid;
            for (ssize_t xoff = -BRIDGE_DIST; xoff <= BRIDGE_DIST; xoff++) {
                for (ssize_t yoff = -BRIDGE_DIST; yoff <= BRIDGE_DIST; yoff++) {
                  size_t test_xid = clampSize(xoff + current_xid,
                                              substrate->width);
                  size_t test_yid = clampSize(yoff + current_yid,
                                              substrate->width);
                    float* pixel = Substrate_getPixel(
                    (struct Substrate*)substrate, test_xid, test_yid);
                    // can direct appear because assign to 1.0f
                    if (*pixel == 1.0f) {
                        float dist_sq = distanceSq(current_xid, current_yid,
                                                 test_xid, test_yid);
                        if (dist_sq < smallest_dist_sq) {
                            smallest_dist_sq = dist_sq;
                            best_bridgeto_xid = test_xid;
                            best_bridgeto_yid = test_yid;
                        }
                    }
                }
            }
            if (smallest_dist_sq == FLT_MAX) { // contour done
                struct Point2d const *start_pt =
                    &contours->points[contour_start];
                struct Point2d const *end_pt =
                    &contours->points[contours->points_length-1];
                float return_dist = distanceSqFlt(start_pt->x, start_pt->y,
                                                  end_pt->x, end_pt->y);
                bool is_closed = return_dist < powf(BRIDGE_DIST, 2.0f);
                struct Contour contour = {
                  contour_start,
                  contours->points_length - contour_start,
                    is_closed
                };
                assert(contours->contours_length < MAX_CONTOURS);
                memcpy(&contours->contours[contours->contours_length++],
                       &contour, sizeof(struct Contour));
                break;
            } else if (smallest_dist_sq != FLT_MAX) { // add a point to contour
                current_xid = best_bridgeto_xid;
                current_yid = best_bridgeto_yid;
                continue;
            }
        }
        // Nothing to do, there are no more (free points) available
    }
    printf("We have a total of %ld points and %ld contours!\n",
        contours->points_length, contours->contours_length);
}

void Contours_deinit(struct Contours *contours) {
    free(contours->points);
}
