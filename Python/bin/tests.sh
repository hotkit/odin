#!/bin/bash
set -eux
cd $(dirname $0)

cd ..
export PYTHONPATH=$(pwd)/odin
cd ..
export ODIN_SCHEMA_PATH=$(pwd)/Schema
pytest ./Python/odin/tests
