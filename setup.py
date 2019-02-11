#!/usr/bin/env python
from distutils.core import setup

datadir = os.path.join('Schema')
datafiles = [(os.path.join('share', 'odin', d), [os.path.join(d,f) for f in files])
    for d, folders, files in os.walk(datadir)]

setup(name='odin',
        version='0.1.6.2',
        description='Odin security system',
        author='Kirt Saelensminde',
        author_email='kirit@proteus-tech.com',
        url='https://kirit.com/Odin/',
        scripts=['Python/bin/odin'],
        packages=['odin'],
        package_dir={'': 'Python'},
        install_requires=['psycopg2-binary'],
        data_files=datafiles,
     )
