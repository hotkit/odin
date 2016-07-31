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
