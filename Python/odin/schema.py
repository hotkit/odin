from odin.cmdline import sql
from odin.connection import OdinSchemaNotPresent
import os.path


MYDIR = os.path.dirname(os.path.abspath(__file__))
SCHEMA_FILES_PATH = [
        '/usr/share/odin/Schema',
        os.path.join(MYDIR, '../../Schema'),
    ]

ADD_MODULE = '''INSERT INTO odin.module VALUES (%s)'''


class SchemaFilesNotFound(Exception):
    pass


def find_schema_path():
    """Searches the locations in the `SCHEMA_FILES_PATH` to
    try to find where the schema SQL files are located.
    """
    for path in SCHEMA_FILES_PATH:
        if os.path.exists(os.path.join(path, 'bootstrap.sql')):
            return path
    raise SchemaFilesNotFound('Searched ' + ':'.join(SCHEMA_FILES_PATH))


def enablemodules(cnx, *modules):
    want = set(modules)
    try:
        got = cnx.load_modules()
        ## As a stop-gap measure we're going to run some SQL here intended
        ## to change the old Odin migration mechanism to the new one. We
        ## need to do this if we're bootstrapping a new one. The SQL needs
        ## to be idempotent as it will be run many times against the same
        ## system
    except OdinSchemaNotPresent:
        cnx.pg.rollback()
        print("Odin not loaded for this database. Bootstrapping...")
        bootstrapfile = os.path.join(find_schema_path(), 'bootstrap.sql')
        sql(cnx, bootstrapfile)
        got = cnx.load_modules()
    for mod in want.difference(got):
        cnx.execute(ADD_MODULE, (mod,))
        print(mod, "enabled")


def migrate(cnx):
    pass

