#include "square.h"
#include "contour.h"

#include <memory.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>
#include <stdio.h>

#define DIST_SAMPLE_RADIUS_SQ 200.0f
#define MIN_CORNER_SEP_DIST_SQ 1000.0f
#define MIN_CONTOUR_LENGTH 30
#define MAX_CONTOUR_LENGTH 10000 

/// This is a misnomer it actually samples by density
static float distanceSample(struct Contours const *contours,
                            struct Contour const* contour, size_t pi) {
    float average_dist = 0;
    for (size_t tpi = contour->start;
        tpi - contour->start < contour->count; tpi++) {
        if (pi != tpi) {
            struct Point2d* current = &contours->points[pi];
            struct Point2d* test = &contours->points[tpi];
            float distance = distanceSqFlt(current->x, current->y,
                test->x, test->y);
            if (distance < DIST_SAMPLE_RADIUS_SQ) {
                average_dist += distance;
            }
        }
    }
    average_dist /= (contour->count - 1);
    return average_dist;
}

void Squares_initFromContours(struct Contours const* contours,
                              struct Squares *squares) {
    squares->squares_length = 0;
    for (size_t ci = 0; ci < contours->contours_length; ci++) {
        // Get the current contour and verify it is valid
        struct Contour const* contour = &contours->contours[ci];
        if (contour->count < MIN_CONTOUR_LENGTH) continue;
        if (contour->count > MAX_CONTOUR_LENGTH) continue;
        if (!contour->is_closed) continue;

        // Populate array of sample distances
        float distances[MAX_CONTOUR_LENGTH]; //~40kib
        for (size_t pi = contour->start; pi - contour->start < contour->count;
            pi++) {
            size_t i = pi - contour->start;
            distances[i] = distanceSample(contours, contour, pi);
        }

        // Find sorted indices of best corner candidates
        struct Square square;

        for (size_t i = 0; i < 4; i++) {
            float best_dist = FLT_MAX;
            for (size_t di = 0; di < contour->count; di++) {
                // Get the current point
                size_t pi = di + contour->start;
                struct Point2d* current_pt = &contours->points[pi];

                // Skip this point if it is too close to an already picked point
                bool is_too_close_to_existant_pt = false;
                for (size_t j = 0; j < i; j++) {
                    struct Point2d* existent_pt = &square.points[j];
                    float dist_to_existent_pt =
                        distanceSqFlt(existent_pt->x, existent_pt->y,
                                      current_pt->x, current_pt->y);
                    if (dist_to_existent_pt < MIN_CORNER_SEP_DIST_SQ) {
                        is_too_close_to_existant_pt = true;
                        break;
                    }
                }
                if (is_too_close_to_existant_pt) continue;

                // Check if this is the best
                float dist = distances[di];
                if (dist < best_dist) {
                    best_dist = dist;
                    memcpy(&square.points[i], &contours->points[pi],
                           sizeof(struct Square));
                }
            }
        }
        // Add the square corners we found
        memcpy(&squares->squares[squares->squares_length++],
               &square, sizeof(struct Square));
    }
}
