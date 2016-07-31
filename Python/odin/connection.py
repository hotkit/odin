import base64
import os
from psycopg2 import connect
from psycopg2.extensions import ISOLATION_LEVEL_SERIALIZABLE


class ModuleNotPresent(Exception):
    """ Thrown when a required module has not been installed.
    """
    pass


class Connection(object):
    def __init__(self, dsn):
        self.pg = connect(dsn)
        self.pg.set_session(isolation_level=ISOLATION_LEVEL_SERIALIZABLE)
        self.cursor = self.pg.cursor()
        self.load_modules()
        self.new_reference()


    def new_reference(self):
        refb = os.urandom(48)
        self.reference = base64.b64encode(refb).decode('utf8')

    def load_modules(self):
        self.cursor.execute("SELECT name FROM odin.module")
        self.modules = set([m[0] for m in self.cursor.fetchall()])


    def assert_module(self, module):
        """ Stops execution with an error if a required module is not installed
            in the system.
        """
        if not self.has_module(module):
            raise ModuleNotPresent(module)

    def has_module(self, module):
        """ Returns true if the module has been installed.
        """
        return module in self.modules


    def execute(self, cmd, *args, **kwargs):
        """ Execute the SQL command and return the data rows as tuples
        """
        self.cursor.execute(cmd, *args, **kwargs)
        return self.cursor.fetchall()


    def commit(self):
        self.pg.commit()
        self.new_reference()

