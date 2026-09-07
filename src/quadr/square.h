#include "contour.h"

struct Square {
    struct Point2d points[4];
};
struct Squares {
    size_t squares_length;
    struct Square squres[MAX_CONTOURS];
};
void Squares_initFromContours(struct Contours const *contours,
    struct Squares* squares);

