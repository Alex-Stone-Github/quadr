from setuptools import setup, Extension
import numpy

quadr = Extension("quadr",
                  sources = [
                      "src/quadr/quadrmod.c",
                      "src/quadr/kernel.c",
                      "src/quadr/contour.c",
                  ],
                  include_dirs=[numpy.get_include()])

setup(
    name = "quadr",
    version = "1.0",
    description = "Some rando desc",
    ext_modules = [quadr],
)
