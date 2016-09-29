

def listing(cnx, command):
    if command == 'users':
        users = cnx.select("SELECT id, is_superuser, full_name FROM odin.identity")
        for (name, su, full) in users:
            print(name, "SUPERUSER" if su else "-", full)
    else:
        print("Unkown listing option", command)

