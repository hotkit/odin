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

## Module dependencies
```mermaid
graph RL

subgraph core
    identity["identity(id)"]

    identity_expiry_ledger["identity_expiry_ledger(reference, identity_id)"]
    identity_expiry_ledger-->identity

    identity_record["identity_record(id)"]
    identity-->identity_record
    merge_record["merge_record(from_identity_id, to_identity_id)"]
    merge_record-->identity_record
    merge_record-->identity_record
end

subgraph app
    app_access_policy["app_access_policy(access_policy)"]

    app_data_sharing_policy["app_data_sharing_policy(data_sharing_policy)"]

    app["app(app_id)"]
    app-->app_access_policy
    app-->app_data_sharing_policy
    app-->identity

    app_role["app_role(app_id, role)"]
    app_role-->app

    app_user_installation_id_ledger["app_user_installation_id_ledger(reference, app_id, identity_id)"]
    app_user_installation_id_ledger-->app_user

    app_user["app_user(app_id, identity_id)"]
    app_user-->app
    app_user-->identity

    app_user_role["app_user_role(app_id, identity_id, role)"]
    app_user_role-->app_role
    app_user_role-->app_user
end

subgraph authn
    credentials["credentials(identity_id)"]
    credentials-->identity

    login_failed["login_failed(username, attempt)"]

    login_success-->credentials
end

subgraph authz
    group["group(slug)"]
    group_membership["group_membership(identity_id, group_slug)"]
    group_membership-->identity
    group_membership-->group

    permission["permission(slug)"]
    group_grant["group_grant(group_slug, permission_slug)"]
    group_grant-->permission
    group_grant-->group
end



subgraph opts/email
    identity_email_ledger["identity_email_ledger(reference, identity_id)"]
    identity_email_ledger-->identity
end

subgraph opts/facebook
    facebook_credentials["facebook_credentials(identity_id)"]
    facebook_credentials-->identity
end

subgraph opts/full-name
    identity_full_name_ledger["identity_full_name_ledger(reference, identity_id)"]
    identity_full_name_ledger-->identity

end

subgraph opts/google
    google_credentials["google_credentials(identity_id)"]
    google_credentials-->identity
end

subgraph opts/installation-id
    identity_installation_id_ledger["identity_installation_id_ledger(reference, identity_id)"]
    identity_installation_id_ledger-->identity
end

subgraph opts/logout
    logout_ledger["logout_ledger(reference, identity_id)"]
    logout_ledger-->identity
end
```
