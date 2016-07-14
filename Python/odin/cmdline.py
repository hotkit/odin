
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

    user:
            user username
        Ensure the requested user is in the system

"""


def makedsn(opts, args):
    dsnargs = {}
    for arg, opt in OPTMAP.items():
        if arg in opts:
            dsnargs[opt] = opts[arg]
    return ' '.join(['%s=%s' % (n, v) for n, v in dsnargs.items()])

