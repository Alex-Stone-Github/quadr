import matplotlib.pyplot as plt
import numpy as np
import cv2
import sys
import quadr

print("I got this form quadr addition, ", quadr.add(2, 3))

print("Opencv2 stuff")
image = cv2.imread("image.png")
if image is None: sys.exit(-1)
#print(image)

print(f"Passing in a np array image of {image.shape}")
output = quadr.takeIn(image)
print(f"I got an output of {output.shape} from my fancy c module!")

print(output)

plt.imshow(image, cmap="gray", vmin=0.0, vmax=255.0)
plt.show()
plt.imshow(output, cmap="gray", vmin=0.0, vmax=1.0)
plt.show()

