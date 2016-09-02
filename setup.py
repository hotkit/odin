#!/usr/bin/env python
from distutils.core import setup

setup(name='odin',
        version='0.1.0',
        description='Odin security system',
        author='Kirt Saelensminde',
        author_email='kirit@proteus-tech.com',
        url='https://www.kirit.com/Odin/',
        packages=['odin'],
        package_dir = {'': 'Python'},
        scripts=['Python/bin/odin'],
     )
