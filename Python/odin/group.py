from psycopg2.extras import Json


SET_GROUP = '''INSERT INTO odin.group_ledger
    (reference, group_slug, description)
    VALUES (%s, %s, %s)
    RETURNING *'''
SET_MEMBERSHIP = '''INSERT INTO odin.group_membership_ledger
    (reference, identity_id, group_slug, member)
    VALUES (%s, %s, %s, %s)
    RETURNING *'''


def setgroup(cnx, group, description=''):
    cnx.assert_module('authz')
    cnx.execute(SET_GROUP, (cnx.reference, group, description))
    print(group, "set up")


def addmembership(cnx, user, *groups):
    cnx.assert_module('authz')
    for group in groups:
        cnx.execute(SET_MEMBERSHIP, (cnx.reference, user, group, True))
        print(user, "is a member of", group)

