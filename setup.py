from setuptools import setup, Extension
import numpy

quadr = Extension("fastqr.quadr",
                  sources = [
                      "fastqr/quadr/quadrmod.c",
                      "fastqr/quadr/kernel.c",
                      "fastqr/quadr/contour.c",
                      "fastqr/quadr/square.c",
                      "fastqr/quadr/pipe.c",
                  ],
                  include_dirs=[numpy.get_include()])

setup(
    name = "fastqr",
    version = "1.0",
    description = "Some rando desc",
    ext_modules = [quadr],
)
