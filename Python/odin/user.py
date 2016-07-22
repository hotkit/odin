from psycopg2.extras import Json


INSERT_USER = '''INSERT INTO odin.identity_ledger (identity_id)
    VALUES (%s)
    ON CONFLICT (identity_id) DO NOTHING
    RETURNING *'''

SET_FULLNAME = '''INSERT INTO odin.identity_full_name_ledger
        (identity_id, full_name)
    VALUES (%s, %s)
    RETURNING *'''
SET_PASSWORD = '''INSERT INTO odin.credentials_password_ledger
        (identity_id, password, process)
    VALUES (%s, %s, %s)
    RETURNING *'''


def createuser(cnx, username, password=None):
    if len(cnx.execute(INSERT_USER, (username,))):
        print(username, "created")
    else:
        print(username, "already present")
    if password:
        setpassword(cnx, username, password)


def setfullname(cnx, username, full_name):
    cnx.assert_module('opt.full-name')
    cnx.execute(SET_FULLNAME, (username, full_name))
    print(username, "full name set")


def setpassword(cnx, username, password):
    cnx.assert_module('authn')
    cnx.execute(SET_PASSWORD, (username, password, Json(dict())))
    print(username, "password set")


class User(object):
    def __init__(self, cnx, username):
        self.cnx = cnx
        self.username = username
