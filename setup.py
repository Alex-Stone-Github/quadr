from setuptools import setup, Extension
import numpy

quadr = Extension("fastqr.quadr",
                  sources = [
                      "src/fastqr/quadr/quadrmod.c",
                      "src/fastqr/quadr/kernel.c",
                      "src/fastqr/quadr/contour.c",
                      "src/fastqr/quadr/square.c",
                      "src/fastqr/quadr/pipe.c",
                  ],
                  include_dirs=[numpy.get_include()])

setup(
    name = "fastqr",
    version = "1.0.0",
    description = "Some rando desc",
    ext_modules = [quadr],
)
