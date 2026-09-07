import matplotlib.pyplot as plt
import numpy as np
import cv2
import sys
import quadr

print("I got this form quadr addition, ", quadr.add(2, 3))

print("Opencv2 stuff & show and image")
image = cv2.imread("image.png")
if image is None: sys.exit(-1)
#plt.imshow(image)
#plt.show()


print(f"Calling the main shebang")
squares = quadr.takeIn(image)


COLOR_RED = (255, 0, 0)
for square in squares:
    for point in square:
        xy = (int(point[0]), int(point[1]))
        cv2.circle(image, xy, 10, COLOR_RED, -1)
    print(f"I got a square: {square}")


plt.imshow(image)
plt.show()
