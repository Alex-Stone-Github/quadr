import typing
import matplotlib.pyplot as plt
import numpy as np
import cv2
import sys
import random
import fastqr

# Open up an image
print("Opencv2 stuff & show and image")
#image = cv2.imread("image2.jpg")
image = cv2.imread("image.png")
if image is None: sys.exit(-1)
plt.imshow(image)
plt.show()

# Call the library dd
print(type(image))
print(f"Calling the main shebang")
squares = fastqr.takeIn(image)
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


