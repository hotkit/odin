from odin.connection import execute_sql_file, OdinSchemaNotPresent
import os


MYDIR = os.path.dirname(os.path.abspath(__file__))

ADD_MODULE = '''INSERT INTO odin.module VALUES (%s)'''


class SchemaFilesNotFound(Exception):
    pass


def _get_schema_files_path():
    env_odin_schema_path = os.getenv('ODIN_SCHEMA_PATH')
    if env_odin_schema_path:
        return env_odin_schema_path.split(os.pathsep)
    else:
        return [
                '/usr/share/odin/Schema',
                os.path.join(MYDIR, '../../Schema'),
            ]
DEFAULT_SCHEMA_FILES_PATH = _get_schema_files_path()


def find_schema_path():
    """Searches the locations in the `SCHEMA_FILES_PATH` to
    try to find where the schema SQL files are located.
    """
    schema_files_path = DEFAULT_SCHEMA_FILES_PATH
    for path in schema_files_path:
        if os.path.exists(os.path.join(path, 'bootstrap.sql')):
            return path
    raise SchemaFilesNotFound('Searched ' + os.pathsep.join(schema_files_path))


def enablemodules(cnx, *modules):
    want = set(modules)
    try:
        got = cnx.load_modules()
    except OdinSchemaNotPresent:
        cnx.pg.rollback()
        print("Odin not loaded for this database. Bootstrapping...")
        bootstrapfile = os.path.join(find_schema_path(), 'bootstrap.sql')
        execute_sql_file(cnx, bootstrapfile)
        got = cnx.load_modules()
    for mod in want.difference(got):
        cnx.execute(ADD_MODULE, (mod,))
        print(mod, "enabled")
    cnx._modules = set()


def migrate(cnx):
    ## As a stop-gap measure we're going to run some SQL here intended
    ## to change the old Odin migration mechanism to the new one. We
    ## need to do this if we're bootstrapping a new one. The SQL needs
    ## to be idempotent as it will be run many times against the same
    ## system
    cnx.execute("UPDATE odin.module "
            "SET name='opts/full-name' "
            "WHERE name='opt.full-name'")
    cnx.execute("UPDATE odin.module "
        "SET name='opts/logout' "
        "WHERE name='opt.logout'")
    cnx.execute("UPDATE odin.migration "
            "SET migration='000-initial.blue.sql' "
            "WHERE module_name='core' AND migration='001-initial.blue.sql'")
    cnx.execute("UPDATE odin.migration "
            "SET migration='002-initial.blue.sql' "
            "WHERE module_name='opts/logout' AND migration='001-initial.blue.sql'")
    ## Find and execute all migration scripts that haven't already been run
    root = find_schema_path()
    scripts = []
    migrations = set(cnx.select("SELECT module_name, migration FROM odin.migration"))
    for dirname, subdirs, files in os.walk(root):
        relpath = os.path.relpath(dirname, root)
        if relpath != '.' and relpath in cnx.load_modules():
            scripts += [(relpath, f) for f in files if f.endswith('.sql')]
    scripts.sort(key=lambda m: m[1])
    for (mod, migration) in scripts:
        if (mod, migration) not in migrations:
            execute_sql_file(cnx, os.path.join(root, mod, migration))

