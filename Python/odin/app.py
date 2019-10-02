from psycopg2.extras import Json

SET_PERMISSION = '''INSERT INTO odin.permission_ledger
    (reference, permission_slug, description)
    VALUES (%s, %s, %s)
    RETURNING *'''

CREATE_APP = """
    INSERT INTO odin.identity_ledger
        (reference, identity_id)
    VALUES (%(reference)s, %(app_id)s);
    INSERT INTO odin.app_ledger (reference, app_id, app_name,
        access_policy, data_sharing_policy, token, redirect_url)
    VALUES (%(reference)s, %(app_id)s, %(app_name)s,
        %(access_policy)s, %(data_sharing_policy)s, %(token)s, %(redirect_url)s)
"""

CREATE_APP_ROLE = """
    INSERT INTO odin.app_role_ledger
        (reference, app_id, role)
    VALUES (%s, %s, %s)
"""

ADD_APP_USER = """
    INSERT INTO odin.app_user_ledger
        (reference, app_id, identity_id, app_user_id)
    VALUES (%s, %s, %s, %s)
"""

ASSIGN_APP_USER_ROLE = """
    INSERT INTO odin.app_user_role_ledger
        (reference, app_id, identity_id, role)
    VALUES (%s, %s, %s, %s)
"""

def createapp(cnx, app_id, app_name, access_policy='INVITE_ONLY', data_sharing_policy='ALL', token=None, redirect_url=None):
    cnx.assert_module('app')
    cnx.execute(CREATE_APP, dict(
        reference=cnx.reference, app_id=app_id,
        app_name=app_name, access_policy=access_policy,
        data_sharing_policy=data_sharing_policy, token=token,
        redirect_url=redirect_url)
    )
    addappuser(cnx, app_id, app_id)
    print('{} app created'.format(app_id))


def createapprole(cnx, app_id, role):
    cnx.assert_module('app')
    cnx.execute(CREATE_APP_ROLE, (cnx.reference, app_id, role))
    print('{}.{} set up'.format(app_id, role))


def addappuser(cnx, app_id, identity_id):
    cnx.assert_module('app')
    cnx.execute(ADD_APP_USER, (cnx.reference, app_id, identity_id, cnx.create_reference()))
    print('{} is an app user of {}'.format(identity_id, app_id))


def assignappuserrole(cnx, app_id, identity_id, role):
    cnx.assert_module('app')
    cnx.execute(ASSIGN_APP_USER_ROLE, (cnx.reference, app_id, identity_id, role))
    print('{} is a member of {}.{}'.format(identity_id, app_id, role))
