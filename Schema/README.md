# Managing database schemas #

These schemas are intended to be run in numerical order with blue before green. Modules run in the order:

1. core
2. authn
3. authz
4. authz-pg

So, run all `001.blue` scripts for the modules you need before you run any `001.green` ones. Then you can move on to `002` etc.

## core ##


## authn ##

Authentication. Primarily to do with ascertaining the identiy of a user. Manages the user's passwords.


## authz ##

Athorization. Primarily to do with ascertaining what a user is allowed to do in the system. Manages user's group membership and the assignment of permissions to groups.


## authz-pg ##

Handles management of user authorization through Postgres roles.


# Schema rationale #

The schema is designed to provide tracking of changes for auditability.

In general application code is expected to write entries into the `leger` tables whose triggers then make the requested change in the underlying data table.


# Notes on specific migrations #

Some migrations will have choices depending on how you want to manage certain features. For example, if you want to use the identity field for the log in name or the user's password.
