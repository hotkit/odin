import csv
from odin.user import createuser, setfullname, setsuperuser


SHORTOPTS = '?d:h:'
PGOPTMAP = {
        '-d': 'dbname',
        '-h': 'host',
    }

HELPTEXT = """Manage an Odin database

    odin [opts] command [args]

opts are one or more of:

    -?                      Print this text
    -h hostname             Postgres host
    -d database             Database  name

comand is one of:

    include:
            include filename
        Find commands (one per line) in the specified file and run them

    sql:
            sql filename
        Load the filename and present the SQL in it to the database for
        execution. This is useful for choosing migrations scripts to run.

    user:
            user username [password]
        Ensure the requested user is in the system. Setting the password
        requires the module `authn`.

    full-name:
            full-name username "Full Name"
        Set the full name field. Requres module `opt/full-name`

    set-superuser:
            superuser username [True|False]
        Sets the superuser bit (defaults to True). Requires the `authz`
        module.
"""


def makedsn(opts, args):
    dsnargs = {}
    for arg, opt in PGOPTMAP.items():
        if arg in opts:
            dsnargs[opt] = opts[arg]
    return ' '.join(["%s='%s'" % (n, v) for n, v in dsnargs.items()])


def include(cnx, filename):
    with open(filename, newline='') as f:
        lines = csv.reader(f, delimiter=' ')
        for line in lines:
            if len(line) and not line[0].startswith('#'):
                command(cnx, *[p for p in line if p])


def sql(cnx, filename):
    with open(filename) as f:
        cmds = f.read()
        cnx.cursor.execute(cmds)
    cnx.load_modules()
    print("Executed", filename)


COMMANDS = {
        'full-name': setfullname,
        'include': include,
        'sql': sql,
        'superuser': setsuperuser,
        'user': createuser,
    }


class UnknownCommand(Exception):
    pass


def command(cnx, cmd, *args):
    if cmd in COMMANDS:
        COMMANDS[cmd](cnx, *args)
    else:
        raise UnknownCommand(cmd)

