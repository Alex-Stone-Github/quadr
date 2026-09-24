import typing
import matplotlib.pyplot as plt
import numpy as np
import cv2
import sys
import random
import quadr

# Test usage with the library
print("I got this form quadr addition, ", quadr.add(2, 3))

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
outimage, corners = quadr.takeIn(image)
print(f"I got back a np array with shape of {outimage.shape}")


colorout = cv2.cvtColor(outimage, cv2.COLOR_GRAY2RGB)
color = (255, 0, 0)

def random_color() -> typing.Tuple[float, float, float]:
    return (random.random(), random.random(), random.random())

for corner in corners:
    x, y = corner
    if x < 0.001 and y < 0.002:
        color = random_color()
    cv2.circle(colorout, (int(x), int(y)), 5, color, -1)


plt.imshow(colorout, cmap="gray")
plt.show()
print(corners)
