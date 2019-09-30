from getopt import gnu_getopt
import pytest
import sys

from odin.app import addappuser, createapp
from odin.connection import Connection
from odin.cmdline import SHORTOPTS, makedsn
from odin.schema import enablemodules, migrate
from odin.user import createuser


def test_addappuser_should_add_app_user():
    optlist, args = gnu_getopt(sys.argv, SHORTOPTS)
    opts = dict(optlist)
    dsn = makedsn(opts, args)
    cnx = Connection(dsn)
    enablemodules(cnx, "authn", "app")
    migrate(cnx)

    createuser(cnx, "user01", "pass1234")
    createapp(cnx, "app_01", "app_01")
    addappuser(cnx, "app_01", "user01")

    app_user = cnx.select("SELECT EXISTS (SELECT 1 FROM odin.app_user WHERE app_id = 'app_01' AND identity_id = 'user01');")
    assert app_user[0][0] == True

    createuser(cnx, "user02", "pass1234")
    addappuser(cnx, "app_01", "user02")

    app_user = cnx.select("SELECT EXISTS (SELECT 1 FROM odin.app_user WHERE app_id = 'app_01' AND identity_id = 'user02');")
    assert app_user[0][0] == True
