# Odin

An authorisation and authentication plug in for Mengmom.


## Build requirements

* A modern C++ compiler (at least C++14).
* Postgres 9.5 (or later). You will need libpq's development package installed.
* The Mengmom web server with the fostgres plug in.

To do a build it's probably easiest to check out Mengmom and start there.


## Modules

Odin is split into [several modules](Schema/README.md) allowing you some flexibility in how to deploy it.


## Configuration and administration

There is a [command line tool](Python/bin/odin.md) for administering and setting up Odin systems. The command line tool can be run with a range of permissions to enable to anything from setting up and configuring new Odin modules to user administration.

