# Odin Python support

Odin contains a simple scripting language that can be used to manage the user database through the `odin` command line. This scripting language is implemented in Python, and as a consequence also provides a Python library that can be used to build Odin features in Python.

The library only supports Python 3.


## Developing on the Odin Python library

You should really use a virtual environment. From the directory where you cloned Odin do something like this:

```bash
cd Python
mkvirtualenv --python=python3 odin
pip install -e ..
```

Run unittest you need prerequisite install:
```bash
apt-get install postgresql
pip install psycopg2
pytest ./Python/odin/tests
```

TODO: python uniitest in stress test
