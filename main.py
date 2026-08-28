import numpy as np
import cv2
import sys
import quadr

#print("I got this form mymod, ", quadr.add(2, 3))
print("This is exclusively from python")

image = cv2.imread("image.png")

if image is None: sys.exit(-1)


print("I am interacting with the c api now")

help(quadr)

output = quadr.takeIn(33, 34)

print(output)

print("Okay I am done now")
