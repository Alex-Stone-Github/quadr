"""
fastqr
~~~~~~

Efficient library for parsing april-tag and qrcode like images
"""

__all__ = [
    "QuadPipe",
    "naive",
    "QuadDetector",
]

from .quadr import QuadPipe, naive
from .detector import QuadDetector, AprilDetector
