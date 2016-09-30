#!/usr/bin/env python
from distutils.core import setup

setup(name='odin',
        version='0.1.3',
        description='Odin security system',
        author='Kirt Saelensminde',
        author_email='kirit@proteus-tech.com',
        url='https://www.kirit.com/Odin/',
        scripts=['Python/bin/odin'],
        packages=['odin'],
        package_dir={'': 'Python'},
        data_files=[
            ('share/odin/Schema', ['Schema/bootstrap.sql']),
            ('share/odin/Schema/authn', ['Schema/authn/001-initial.blue.sql']),
            ('share/odin/Schema/authz', ['Schema/authz/001-initial.blue.sql']),
            ('share/odin/Schema/core', ['Schema/core/001-initial.blue.sql']),
            ('share/odin/Schema/opts/full-name', ['Schema/opts/full-name/001-initial.blue.sql']),
        ],
     )
