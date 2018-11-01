#!/usr/bin/env python
from distutils.core import setup

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
        data_files=[
            ('share/odin/Schema', ['Schema/bootstrap.sql']),
            ('share/odin/Schema/authn', ['Schema/authn/001-initial.blue.sql']),
            ('share/odin/Schema/authz', [
                'Schema/authz/001-initial.blue.sql',
                'Schema/authz/002-view-user_permission.blue.sql']),
            ('share/odin/Schema/core', ['Schema/core/000-initial.blue.sql']),
            ('share/odin/Schema/opts/full-name', ['Schema/opts/full-name/001-initial.blue.sql']),
            ('share/odin/Schema/opts/email', ['Schema/opts/email/001-initial.blue.sql']),
            ('share/odin/Schema/opts/forgotten-password', ['Schema/opts/forgotten-password/001-initial.blue.sql']),
            ('share/odin/Schema/opts/facebook', ['Schema/opts/facebook/001-initial.blue.sql']),
            ('share/odin/Schema/opts/google', ['Schema/opts/google/001-initial.blue.sql']),
            ('share/odin/Schema/opts/installation-id', [
                'Schema/opts/installation-id/001-initial.blue.sql'
            ]),
            ('share/odin/Schema/app', ['Schema/app/002-initial.blue.sql']),
            ('share/odin/Schema/opts/logout', [
                'Schema/opts/logout/002-initial.blue.sql',
                'Schema/opts/logout/003-fix-logout-count.blue.sql']),
        ],
     )
