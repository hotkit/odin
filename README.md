# Odin #

An authorisation and authentication plug in for Mengmom.


## Requirements ##

* A modern C++ compiler (at lesat C++14).
* Postgres 9.5. You will need libpq's development package installed.
* The Mengmom web server with the fostgres plug in.

To do a build it's probably easiest to check out Mengmom and start there telling it you want to build odin:

    mengmom/compile mengmom odin

See the Mengmom README for full details.


## Modules ##

Odin is split into several module allowing you some flexibility in how to deploy it.

