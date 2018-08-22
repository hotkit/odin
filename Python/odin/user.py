import base64
from getpass import getpass
import hashlib
import os
from psycopg2.extras import Json


INSERT_USER = '''INSERT INTO odin.identity_ledger
        (reference, identity_id)
    VALUES (%s, %s)'''
SET_FULLNAME = '''INSERT INTO odin.identity_full_name_ledger
        (reference, identity_id, full_name)
    VALUES (%s, %s, %s)'''
SET_PASSWORD = '''INSERT INTO odin.credentials_password_ledger
        (reference, identity_id, password, process)
    VALUES (%s, %s, %s, %s)'''
EXPIRE_USER = '''INSERT INTO odin.identity_expiry_ledger
        (reference, identity_id, expires)
    VALUES (%s, %s, %s)'''
SELECT_EXPIRY = '''SELECT expires FROM odin.identity WHERE id=%s'''
SET_SUPERUSER = '''INSERT INTO odin.identity_superuser_ledger
        (reference, identity_id, superuser, annotation)
    VALUES (%s, %s, %s, %s)'''


def createuser(cnx, username, password=None):
    cnx.execute(INSERT_USER, (cnx.reference, username))
    print(username, "set up")
    if password:
        print("WARNING: Using createuser (Python) or user (Odin script) to set a password is deprecated")
        print("Instead use a separate call to setpassword (Python) or password (Odin script).")
        setpassword(cnx, username, password)


def expireuser(cnx, username, expire=None):
    cnx.assert_module('authn')
    if expire is None:
        expires = cnx.select("SELECT now()")[0][0]
        cnx.execute(EXPIRE_USER, (cnx.reference, username, expires))
        print(username, "account expired at", expires)
    elif expire == "never":
        cnx.execute(EXPIRE_USER, (cnx.reference, username, None))
        print(username, "account no longer expires")
    else:
        cnx.execute(EXPIRE_USER, (cnx.reference, username, expire))
        expires = cnx.select(SELECT_EXPIRY, (username,))[0][0]
        print(username, "account expires at", expires)


def setfullname(cnx, username, full_name):
    cnx.assert_module('opts/full-name')
    cnx.execute(SET_FULLNAME, (cnx.reference, username, full_name))
    print(username, "full name set")


def setpassword(cnx, username, password=None):
    cnx.assert_module('authn')
    if not password:
        p1 = getpass()
        p2 = getpass()
        while p1 != p2:
            print("Passwords do not match")
            p1 = getpass()
            p2 = getpass()
        password = p1
    salt = os.urandom(24)
    process = dict(name='pbkdf2-sha256', rounds=300000, length=32,
        salt=base64.b64encode(salt).decode('utf8'))
    pwhash = hashlib.pbkdf2_hmac('sha256', password.encode('utf8'), salt, 300000)
    cnx.execute(SET_PASSWORD, (cnx.reference, username,
        base64.b64encode(pwhash).decode('utf8'), Json(process)))
    print(username, "password set")


def setsuperuser(cnx, username, su=True, annotation=dict()):
    cnx.assert_module('authz')
    cnx.execute(SET_SUPERUSER, (cnx.reference, username, su, Json(annotation)))
    print(username, "super user set" if su else "unpriviliged user")


class User(object):
    def __init__(self, cnx, username):
        self.cnx = cnx
        self.username = username

