from distutils.core import setup
from Cython.Build import cythonize

setup(
    ext_modules=cythonize("__pycache__/Display.cpython-37.pyx"),
)