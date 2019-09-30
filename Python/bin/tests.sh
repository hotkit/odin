#!/bin/bash
set -eux
cd $(dirname $0)

PYTHONPATH=$(pwd)/../odin ODIN_SCHEMA_PATH=$(pwd)/../../Schema PGDBNAME=test_odin_python pytest ../odin/tests
