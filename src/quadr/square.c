#include "square.h"
#include "contour.h"

#include <memory.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>

#define MOUND_THRESH 1.00f
#define SQ_CORNERS 4
void Squares_initFromContours(struct Contours const *contours,
                              struct Squares *squares) {
    squares->squares_length = 0;
    for (size_t ci = 0; ci < contours->contours_length; ci++) {
        struct Contour const* contour = &contours->contours[ci];
        struct Square square;
        size_t sq_points_length = 0;
        puts("Analyzing a contour #######################################33");
        for (size_t pi = contour->start; pi - contour->start < contour->count;
             pi++) {
            size_t li = pi - 1;
            if (li < contour->start) li += contour->count;
            size_t ni = contour->start +
                ((pi - contour->start + 1) % contour->count);
            float last_mound =
                powf(contours->points_changes[li].thetadot, 2.0f);
            float current_mound =
                powf(contours->points_changes[pi].thetadot, 2.0f);
            float next_mound =
                powf(contours->points_changes[ni].thetadot, 2.0f);

            bool is_peak = current_mound > last_mound &&
                           current_mound > next_mound &&
                           current_mound > MOUND_THRESH;

            printf("To analyze (%f, %f) theta %f mound %f %d\n",
                   contours->points[pi].x, contours->points[pi].y, 
                   contours->points_changes[pi].theta, current_mound, is_peak);

            if (is_peak) {
                printf("This is a peak, I am adding a corner with the preious point\n");
                // Can't have more than 4 corners
                if (sq_points_length >= SQ_CORNERS) break;
                struct Point2d* point = &contours->points[pi];
                memcpy(&square.points[sq_points_length++], point,
                    sizeof(struct Point2d));
            }
        }
        if (sq_points_length == 4 && contour->is_closed) {
            printf("########################3######3ADDING a square!\n");
            assert(squares->squares_length < MAX_CONTOURS);
            // TODO: Perform a squareness test
            memcpy(&squares->squres[squares->squares_length++],
                   &square, sizeof(struct Square));
        }
    }
}
