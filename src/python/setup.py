import sys
import os

if not os.path.exists("phoneutils/phoneutils.c_phoneutils.c"):
    try:
        import Cython
    except ImportError:
        raise SystemExit("You need Cython -- http://cython.org/")
    try:
        import Pyrex
    except ImportError:
        raise SystemExit(
            "You need Pyrex -- "
            "http://www.cosc.canterbury.ac.nz/greg.ewing/python/Pyrex/")

from distutils.core import setup
from setuptools import find_packages
from distutils.extension import Extension
from Cython.Distutils import build_ext
import commands

def pkgconfig(*packages, **kw):
    flag_map = {'-I': 'include_dirs', '-L': 'library_dirs', '-l': 'libraries'}
    pkgs = ' '.join(packages)
    cmdline = 'pkg-config --libs --cflags %s' % pkgs

    status, output = commands.getstatusoutput(cmdline)
    if status != 0:
        raise ValueError("could not find pkg-config module: %s" % pkgs)

    for token in output.split():
        if flag_map.get(token[:2]):
            kw.setdefault(flag_map.get(token[:2]), []).append(token[2:])
        elif token.startswith("-Wl,"):
            kw.setdefault("extra_link_args", []).append(token)
        else:
            kw.setdefault("extra_compile_args", []).append(token)
    return kw

phoneutils_mod = Extension('phoneutils.c_phoneutils',
                       sources=['phoneutils/c_phoneutils.pyx'],
                       **pkgconfig('"libphone-utils"'))

setup(
    name = 'python-phoneutils',
    version = '0.1',
    license = 'LGPL',
    author = 'Sebastian Krzyszkowiak',
    author_email = 'seba.dos1@gmail.com',
    url='http://shr-project.org/',
    description = 'Python bindings for libphone-utils',
    long_description = '',
    keywords = 'wrapper bindings phone utils telephone numbers',
    packages = find_packages(),
 #   zip_safe=False,
    cmdclass = {"build_ext": build_ext},
    ext_modules = [phoneutils_mod]
)
