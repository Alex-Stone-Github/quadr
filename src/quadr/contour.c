#include "contour.h"
#include "kernel.h"

#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

#define THETA_LOOKAHEAD 5
#define BRIDGE_DIST 2.0f

void Contours_initFromSubstrate(struct Substrate const *substrate,
                                struct Contours *contours) {
    // We can have at a maximum 1 point per pixel
    size_t pixel_count = substrate->width * substrate->height;
    contours->points_capacity = pixel_count;
    contours->points = calloc(pixel_count, sizeof(struct Contour));
    contours->points_changes = calloc(pixel_count,
        sizeof(struct ContourPointMeta));
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
static float distanceSqFlt(float ax, float ay, float bx, float by) {
    return powf(ax - bx, 2.0f) + powf(ay - by, 2.0f);
}
static float distanceSq(size_t ax, size_t ay, size_t bx, size_t by) {
    return distanceSqFlt((float)ax, (float)ay, (float)bx, (float)by);
}
struct ContoursPointIter {
    struct Contours* contours;
    size_t ci, pi;
};
struct ContoursPointIterIdxs {
    size_t current;
    size_t next;
    size_t look;
};
void ContoursPointIter_init(struct ContoursPointIter *iter,
    struct Contours* contours) {
    iter->contours = contours;
    iter->ci = 0;
    iter->pi = 0;
}
bool ContoursPointIter_next(struct ContoursPointIter *iter,
                            struct ContoursPointIterIdxs* idxs) {
    // Resolve our current place
    if (iter->pi - iter->contours->contours[iter->ci].start ==
        iter->contours->contours[iter->ci].count) {
        iter->ci ++;
        iter->pi = iter->contours->contours[iter->ci].start;
    }
    if (iter->ci == iter->contours->contours_length) return false;
    struct Contour* contour = &iter->contours->contours[iter->ci];

    // Construct our current indexes
    idxs->current = iter->pi;
    idxs->next = contour->start +
        ((iter->pi - contour->start + 1) % contour->count);
    idxs->look = contour->start +
        ((iter->pi - contour->start + THETA_LOOKAHEAD) % contour->count);

    iter->pi ++;
    return true;
}
/// Theroretically, I should only be using this function for x <= 10ish
float smallSqrtApprox(float x) {
    assert(x < 10.0f); // Too big x for approx to work
    return sqrtf(x); // opt out
    if (x <= 0) return 0.0f;

    // Newtons method for approximation (4 iters)
    float guess = 2.0f;
    guess = 0.5f * (guess + (x / guess));
    guess = 0.5f * (guess + (x / guess));
    guess = 0.5f * (guess + (x / guess));
    guess = 0.5f * (guess + (x / guess));
    return guess;
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
                struct Contour contour = {
                    contour_start, contours->points_length - contour_start
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

    // Get distance squared and thetas
    printf("Fa la la la la - making contour data and derivatives\n");
    struct ContoursPointIter pointIter;
    ContoursPointIter_init(&pointIter, contours);
    struct ContoursPointIterIdxs idxs;
    while (ContoursPointIter_next(&pointIter, &idxs)) {
        float ds_sq = distanceSqFlt(contours->points[idxs.current].x,
                                    contours->points[idxs.current].y,
                                    contours->points[idxs.next].x,
                                    contours->points[idxs.next].y
                                    );
        float ds = smallSqrtApprox(ds_sq);
        if (ds == 0.0f) ds += 0.001f; // ignore zero problem
        float look_dx = contours->points[idxs.look].x -
            contours->points[idxs.current].x;
        float look_dy = contours->points[idxs.look].y -
            contours->points[idxs.current].y;
        float look_theta = atan2f(look_dy, look_dx);
        while (look_theta <= 0.0f) look_theta += 2.0f * M_PI;
        //printf("%f, %f, %f, %f\n", look_dx, look_dy, look_o, look_theta);
        // calculate theta beter

        // Use an approximation for the sqrt to save time with large mtn of
        contours->points_changes[idxs.current].ds = ds;
        contours->points_changes[idxs.current].theta = look_theta;
        // Classify them as closed or not too
        if (idxs.current + 1 != idxs.next && ds <= BRIDGE_DIST)
            contours->contours[pointIter.ci].is_closed = true;
        else
            contours->contours[pointIter.ci].is_closed = false;
    }
    // Take theta derivative
    ContoursPointIter_init(&pointIter, contours);
    while (ContoursPointIter_next(&pointIter, &idxs)) {
        // Use a sqrt approximation to save time with large amnt of sqrts
        float theta = contours->points_changes[idxs.current].theta; 
        float theta_h = contours->points_changes[idxs.look].theta;
        float ds = contours->points_changes[idxs.current].ds;
        contours->points_changes[idxs.current].thetadot = (theta_h - theta)/ds;
    }
}

void Contours_deinit(struct Contours *contours) {
    free(contours->points);
    free(contours->points_changes);
}
