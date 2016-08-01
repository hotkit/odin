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

# Only three groups to start with
group auditor "Can view most of the user and group set up and audit trails in the system"
group admin-user "Can create users and assign groups to them"

# Various permissions are needed
permission create-user "Can create a user"
permission create-group "Can create a group"

# Assign permissions to groups
assign admin-group create-group
assign admin-user create-user

# Assign group memberships
membership anonymous auditor

