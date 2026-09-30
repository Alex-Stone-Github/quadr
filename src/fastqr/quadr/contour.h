#pragma once

#include "kernel.h"

#include <stdio.h>
#include <stdbool.h>

struct Point2d {
    float x, y;
};
struct Contour {
    // Includes index of start and end exc
    size_t start;
    size_t count;
    bool is_closed;
};
struct ContourPointMeta {
    float ds;
    float theta;
    float thetadot;
};
#define MAX_CONTOURS 500
// This is a contour allocator, it keeps track of contours from a substrate
struct Contours {
    // points
    size_t points_capacity;
    size_t points_length;
    struct Point2d* points;
    //struct ContourPointMeta* points_changes;

    // contours
    size_t contours_length;
    struct Contour contours[MAX_CONTOURS];
};
void Contours_initFromSubstrate(struct Substrate const *substrate,
    struct Contours* contours);
void Contours_findContoursConsumeSubstrate(struct Contours* contours,
    struct Substrate const* substrate);
void Contours_deinit(struct Contours* contours);

float distanceSqFlt(float ax, float ay, float bx, float by);
