INSERT_USER = '''INSERT INTO odin.identity_ledger VALUES (%s)
    ON CONFLICT (identity_id) DO NOTHING
    RETURNING *'''


def createuser(cnx, username, password=None):
    if len(cnx.execute(INSERT_USER, (username,))):
        print(username, "created")
    else:
        print(username, "already present")

