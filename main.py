import typing
import matplotlib.pyplot as plt
import numpy as np
import cv2
import sys
import random
import fastqr
from fastqr.detector import AprilDetector
from fastqr.squares import cvt_cords


# Open up an image
print("Opencv2 stuff & show and image")
#image = cv2.imread("image2.jpg")
image = cv2.imread("image.png")
if image is None: sys.exit(-1)
plt.imshow(image)
plt.show()

"""
# Call the library dd
print(type(image))
print(f"Calling the main shebang")
squares = fastqr.naive(image)
print(f"I got back a np array with shape of {squares.shape}")


def random_color() -> typing.Tuple[float, float, float]:
    randchan = lambda : random.randint(0, 256)
    return (randchan(), randchan(), randchan())

for square in squares:
    color = random_color()
    for corner in square:
        x, y = corner
        cv2.circle(image, (int(x), int(y)), 5, color, -1)

plt.imshow(image)
plt.show()

# Some more stuff
print("Doing type test")

mything = fastqr.quadr.QuadPipe(image.shape[1], image.shape[0])
print(mything)
mything.print_stats()
print(mything.process(image).shape)

print("Done tyep test")
"""

print("Using erganomics")

with fastqr.QuadDetector(image.shape[1], image.shape[0]) as qd:
    squares = qd.find_squares(image)
    print(f"I found {len(squares)} squares")
    for square in squares:
        for pt in square:
            cv2.circle(image, (pt.x, pt.y), 5, (255, 0, 0), -1)
        print(square)

print("Finally done")

print ("Detecting april tags")
with fastqr.AprilDetector(image.shape[1], image.shape[0]) as april:
    pts = april.find_tags(image)

plt.imshow(image)
plt.show()
