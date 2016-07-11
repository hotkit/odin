# Managing database schemas #

These schemas are intended to be run in numerical order with blue before green. Modules run in the order:

1. core
2. authn
3. authz

So, run all `001.blue` scripts for the modules you need before you run any `001.green` ones. Then you can move on to `002` etc.

