sql ../Schema/core/001-initial.blue.sql
sql ../Schema/authn/001-initial.blue.sql
sql ../Schema/authz/001-initial.blue.sql
sql ../Schema/authz/002-view-user_permission.blue.sql
sql ../Schema/opts/full-name/001-initial.blue.sql
sql ../Schema/opts/logout/001-initial.blue.sql

# Create the initial super user account
user root password123
full-name root "System admin"
superuser root

# Create an account to be used by anonymous users
user anonymous

# Only three groups to start with
group testers "Test users"

# Various permissions are needed
permission run-test "Allowed to run a test"

# Assign permissions to groups
assign testers run-test

# Create some test users

user fred password123
full-name fred "Fred Flintstone"
membership fred auditor admin-group admin-user

user barney password123
full-name barney "Barney Rubble"
membership barney testers auditor admin-user

