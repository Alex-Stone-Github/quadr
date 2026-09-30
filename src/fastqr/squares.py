""" This module contains the square detection wrapper of fastqr """

from dataclasses import dataclass
import numpy as np
import typing
import math
import itertools

from .quadr import QuadPipe

class Point2d(typing.NamedTuple):
    x: int
    y: int

def distance(a: Point2d, b: Point2d) -> float:
    return math.sqrt((a.x - b.x) ** 2 + (a.y - b.y) ** 2)
def angle_to(origin: Point2d, cast: Point2d) -> float:
    theta = math.atan2((cast.y - origin.y), (cast.x - origin.x))
    # put theta within -pi to pi
    theta -= math.pi * 2
    while theta < -math.pi: theta += math.pi * 2
    return theta

"""This is a tuple of points start at the top left corner in clockwise order"""
Square = typing.Tuple[Point2d, Point2d, Point2d, Point2d]

def cvt_cords(square: Square, u: float, v: float) -> Point2d:
    top = Point2d(square[1].x - square[0].x, square[1].y - square[0].y)
    left = Point2d(square[3].x - square[0].x, square[3].y - square[0].y)
    u_vec = Point2d(int(top.x * u), int(top.y * u))
    v_vec = Point2d(int(left.x * v), int(left.y * v))
    return Point2d(u_vec.x+v_vec.x+square[0].x, u_vec.y+v_vec.y+square[0].y)

def is_square(square: Square, thresh: float = 0.02) -> bool:
    side_lens = [
        distance(square[0], square[1]),
        distance(square[1], square[2]),
        distance(square[2], square[3]),
        distance(square[3], square[0]),
    ]
    avg_side_len = sum(side_lens) / len(side_lens)
    cross_lens = [
        distance(square[0], square[2]),
        distance(square[1], square[3]),
    ]
    avg_cross_len = sum(cross_lens) / len(cross_lens)
    side_len_ratio = avg_cross_len / avg_side_len
    return abs(side_len_ratio - math.sqrt(2)) < thresh

class QuadDetector:
    def __init__(self, width: int, height: int):
        assert width > 50 and height > 50, "Image should have a reasonable size"
        self._width = width
        self._height = height
        self._pipeline = None

    # Resource Management
    def create_pipeline(self):
        """ Allocate my image quad detection pipeline """
        self._pipeline = QuadPipe(self._width, self._height)
    def __enter__(self) -> typing.Self:
        self.create_pipeline()
        return self
    def destroy_pipeline(self):
        """ Free my image quad detection pipeline """
        self._pipeline = None
    def __exit__(self, *_) -> bool:
        self.destroy_pipeline()
        return False

    # Quadrilateral detection
    def find_quads(self, image: np.array) -> np.array:
        """ Find identifiable quadrilateral shapes in an image """
        assert isinstance(self._pipeline, QuadPipe), """
            You must create the pipeline before detecting quads!
            Consider calling `self.create_pipeline()` first"""
        return self._pipeline.process(image)
    def find_squares(self, image: np.array) -> typing.List[Square]:
        quads = self.find_quads(image)
        squares = []
        for quad in quads:
            cenx, ceny = np.mean(quad, axis=0)
            center = Point2d(int(cenx), int(ceny))

            points = list(map(lambda pt: Point2d(int(pt[0]), int(pt[1])), quad))
            points.sort(key = lambda pt: angle_to(center, pt))
            square = tuple(points)
            if is_square(square):
                squares.append(square)

        return squares
