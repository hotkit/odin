# Enable the modules we want and migrate them all
enable-modules authn authz opts/full-name opts/logout opts/email opts/forgotten-password opts/facebook opts/google app opts/installation-id
migrate

# Create the initial super user account
user root
password root password123
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

# Create test app
create-app bowling_app "Bowling Game" OPEN ALL "shhhh" "www.bowling-game.com/redirect"

# Create test roles
create-app-role bowling_app "player"
create-app-role bowling_app "janitor"

add-app-user bowling_app fred
add-app-user bowling_app barney

assign-app-user-role bowling_app fred player
assign-app-user-role bowling_app barney janitor
