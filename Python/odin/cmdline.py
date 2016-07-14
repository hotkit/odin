from odin.user import createuser


SHORTOPTS = '?d:h:'
OPTMAP = {
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
            user username
        Ensure the requested user is in the system

"""


def makedsn(opts, args):
    dsnargs = {}
    for arg, opt in OPTMAP.items():
        if arg in opts:
            dsnargs[opt] = opts[arg]
    return ' '.join(["%s='%s'" % (n, v) for n, v in dsnargs.items()])


def include(cnx, filename):
    with open(filename) as f:
        lines = f.readlines()
        for line in lines:
            command(cnx, *[l.strip() for l in line.split()])


def sql(cnx, filename):
    with open(filename) as f:
        cmds = f.read()
        cnx.cursor.execute(cmds)
    cnx.load_modules()
    print("Executed", filename)


COMMANDS = dict(include=include, sql=sql, user=createuser)


class UnknownCommand(Exception):
    pass


def command(cnx, cmd, *args):
    if cmd in COMMANDS:
        COMMANDS[cmd](cnx, *args)
    else:
        raise UnknownCommand(cmd)

