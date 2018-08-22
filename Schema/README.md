# Managing database schemas


Before any of the modules can be loaded the `bootstrap.sql` must be run in the database you want to put the Odin data into.

Individual migration files can be easily run using the Python `odin` command.


# Enabling modules and migrating the schema

Modules should be enabled using the `odin enable-modules` command:

    odin enable-modules mod1 [mod2 [mod3 ...]]

Enabling a module does not run its migrations and the core module is always enabled whether or not it is listed.

Whenever fetching a new version of Odin its migrations need to be run. This is done using the `odin migrate` command.

    odin migrate

It will report any migration scripts that it executes.


# The Modules

## core ##

Central management of the identity of the users of a system. This module is always enabled.


## authn ##

Authentication. Primarily to do with ascertaining the identiy of a user. Manages the user's passwords.


## authz ##

Athorization. Primarily to do with ascertaining what a user is allowed to do in the system. Manages user's group membership and the assignment of permissions to groups.

## app ##

Application. Added application mechanism to Odin, user can login with desire app_id and get specific JWT token for that app.


## opts ##

There are various optional modules under opts.

### full-name ###

Adds a full name field to the identity.

### logout ###

Adds a log out count which is used to verify JWT. This allows an account to log out and revoke all JWTs, but means that the Odin database must be accessed in order to verify that a JWT is valid. This module requires the `authn` module to be loaded.


# Schema rationale #

The schema is designed to provide tracking of changes for auditability. It is also designed to enforce as much as possible of the data storage rules.

In general application code is expected to write entries into the `ledger` tables whose triggers then make the requested change in the underlying data table.


