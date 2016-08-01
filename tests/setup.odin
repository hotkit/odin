sql ../Schema/core/001-initial.blue.sql
sql ../Schema/authn/001-initial.blue.sql
sql ../Schema/authz/001-initial.blue.sql
sql ../Schema/opts/full-name/001-initial.blue.sql

# Create the initial super user account
user root password123
full-name root "System admin"
superuser root

# Create an account to be used by anonymous users
user anonymous

group auditor "Can view most of the user and group set up and audit trails in the system"
group admin-group "Can create groups and assign permissions to them"
group admin-user "Can create users and assign groups to them"
