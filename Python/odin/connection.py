import base64
from datetime import datetime, timezone
from getpass import getpass
import os
import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_SERIALIZABLE


EPOCH = datetime(1970, 1, 1, tzinfo=timezone.utc)


class OdinSchemaNotPresent(Exception):
    """Thrown if we can't load the list of modules because the `odin.modules`
    table doesn't exist.
    """
    pass


class ModuleNotPresent(Exception):
    """ Thrown when a required module has not been installed.
    """
    pass


class Connection(object):
    def __init__(self, dsn):
        try:
            self.pg = psycopg2.connect(dsn)
        except psycopg2.OperationalError as e:
            if "fe_sendauth" in str(e):
                pwd = getpass()
                dsn = dsn + " password='%s'" % pwd
                self.pg = psycopg2.connect(dsn)
            else:
                raise e
        self.pg.set_session(isolation_level=ISOLATION_LEVEL_SERIALIZABLE)
        self.cursor = self.pg.cursor()
        self._modules = set()
        self.new_reference()


    def new_reference(self):
        self.reference = self.create_reference()
        self.cursor.execute("""SET LOCAL odin.reference=%s;""", (self.reference,))


    def create_reference(self):
        epoch_time = (datetime.now(timezone.utc) - EPOCH).total_seconds()
        ref = base64.b64encode(os.urandom(3)).decode('utf8')
        return "%s-%s" % (epoch_time, ref)


    def load_modules(self):
        if not self._modules:
            try:
                self.cursor.execute("SELECT name FROM odin.module")
                self._modules = set([m[0] for m in self.cursor.fetchall()])
            except psycopg2.ProgrammingError as e:
                if e.pgcode == "42P01":
                    raise OdinSchemaNotPresent()
                else:
                    raise
        return self._modules

    def assert_module(self, module):
        """ Stops execution with an error if a required module is not installed
            in the system.
        """
        if not self.has_module(module):
            raise ModuleNotPresent(module)

    def has_module(self, module):
        """ Returns true if the module has been installed.
        """
        return module in self.load_modules()


    def execute(self, cmd, *args, **kwargs):
        """ Execute the SQL command and return the data rows as tuples
        """
        self.cursor.execute(cmd, *args, **kwargs)


    def select(self, cmd, *args, **kwargs):
        """ Execute the SQL command and return the data rows as tuples
        """
        self.cursor.execute(cmd, *args, **kwargs)
        return self.cursor.fetchall()


    def commit(self):
        self.pg.commit()
        self.new_reference()


def execute_sql_file(cnx, filename):
    try:
        with open(filename) as f:
            cmds = f.read()
            cnx.cursor.execute(cmds)
        cnx.load_modules()
        print("Executed", filename)
    except Exception:
        print("Error whilst running", filename)
        raise
