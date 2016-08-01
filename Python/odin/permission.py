from psycopg2.extras import Json


SET_PERMISSION = '''INSERT INTO odin.permission_ledger
    (reference, permission_slug, description)
    VALUES (%s, %s, %s)
    RETURNING *'''


def setpermission(cnx, permission, description=''):
    cnx.assert_module('authz')
    cnx.execute(SET_PERMISSION, (cnx.reference, permission, description))
    print(permission, "set up")

