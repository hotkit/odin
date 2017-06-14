import csv
from odin.display import listing
from odin.group import (addmembership, assignpermission, removemembership,
    setgroup)
from odin.permission import setpermission
from odin.user import (createuser, expireuser, setfullname, setpassword,
    setsuperuser)


SHORTOPTS = '?d:h:p:U:'
PGOPTMAP = {
        '-d': 'dbname',
        '-h': 'host',
        '-p': 'port',
        '-U': 'user',
    }

HELPTEXT = """Manage an Odin database

    odin [opts] command [args]

opts are one or more of:

    -?                      Print this text
    -h hostname             Postgres host
    -d database             Database  name
    -U username             Database username

Comand is one of:

    assign group permission1 [permission2 [permission3 ...]]
        Assign one or more permissions to a group.

    exclude username group1 [group2 [group3 ...]]
        Remove the user from the specified groups. Requires the `authz`
        module.

    expire username [time-date|never]
        Expire the identity for the user at this time.

    full-name username "Full Name"
        Set the full name field. Requres module `opt/full-name`

    help
        Show this text

    group name [description]
        Set up a group and its description.

    include filename
        Find commands (one per line) in the specified file and run them

    list [users|user-groups|user-permissions]
        List information about the current configuration of the system.

    membership user group1 [group2 [group3 ...]]
        Add the user to one or more groups. Requires the `authz` module.

    password name [password]
        Set (or reset) the user's password. If the password is not provided
        as part of the command then the tool will prompt the user to
        enter one. Setting the password requires the module `authn`.

    permission name [description]
        Set up a permission and its description.

    sql filename
        Load the filename and present the SQL in it to the database for
        execution. This is useful for choosing migrations scripts to run.

    user username [password]
        Ensure the requested user is in the system. Setting the password
        requires the module `authn`.

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
        'assign': assignpermission,
        'exclude': removemembership,
        'expire': expireuser,
        'full-name': setfullname,
        'group': setgroup,
        'include': include,
        'list': listing,
        'membership': addmembership,
        'password': setpassword,
        'permission': setpermission,
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

