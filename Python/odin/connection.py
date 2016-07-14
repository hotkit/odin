from psycopg2 import connect
from psycopg2.extensions import ISOLATION_LEVEL_SERIALIZABLE


class ModuleNotPresent(Exception):
    pass


class Connection(object):
    def __init__(self, dsn):
        self.pg = connect(dsn)
        self.pg.set_session(isolation_level=ISOLATION_LEVEL_SERIALIZABLE)
        self.cursor = self.pg.cursor()
        self.load_modules()


    def load_modules(self):
        self.cursor.execute("SELECT name FROM odin.module")
        self.modules = set([m[0] for m in self.cursor.fetchall()])


    def assert_module(self, module):
        if not module in self.modules:
            raise ModuleNotPresent(module)


    def execute(self, cmd, *args, **kwargs):
        """Execute the SQL command and return the data rows as tuples
        """
        self.cursor.execute(cmd, *args, **kwargs)
        return self.cursor.fetchall()


    def commit(self):
        self.pg.commit()

