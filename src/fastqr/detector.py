"""
This file contains all of the detectors that can be used in fastqr
"""

import typing
import math
import itertools
import numpy as np

from .squares import QuadDetector, Square, distance, angle_to, cvt_cords

class AprilDetector(QuadDetector):
    """ This is a specification of a quad detector for detecting april tags """
    def find_tags(self, image: np.array) -> typing.List[Point2d]:
        for square in self.find_squares(image):
            binary = []
            WIDTH_CELLS = 8
            sample_places = np.linspace(3/(WIDTH_CELLS*2), 13/(WIDTH_CELLS*2),
                                        WIDTH_CELLS - 2)
            # Y
            for v in sample_places:
                # X
                for u in sample_places:
                    pt = cvt_cords(square, u, v)
                    pixel = bool(np.mean(image[pt.y, pt.x]) > 0.5)
                    color = (100, 100, 255)
                    if pixel:
                        color = (100, 255, 100)
                    binary.append(pixel)
                    import cv2
                    cv2.circle(image, (pt.x, pt.y), 5, color, -1)

            for v in binary:
                print(1 if v else 0, end = "")
            print()
