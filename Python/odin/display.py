

def listing(cnx, command):
    if command == 'users':
        users = cnx.select("SELECT id, is_superuser, full_name FROM odin.identity")
        for (name, su, full) in users:
            print(name, "SUPERUSER" if su else "-", full)
    elif command == 'user-groups':
        ug = cnx.select("SELECT DISTINCT id, group_slug, odin.group.description \
            FROM odin.identity \
            JOIN odin.group_membership ON \
                (odin.group_membership.identity_id=odin.identity.id OR odin.identity.is_superuser) \
            JOIN odin.group ON (odin.group_membership.group_slug = odin.group.slug) \
            ORDER BY id ")
        pu = None
        for (name, group, description) in ug:
            if pu != name:
                print(name)
                pu = name
            print("   ", group, description)
    elif command == 'user-permissions':
        up = cnx.select("SELECT identity_id, permission_slug, description \
            FROM odin.user_permission \
            ORDER BY identity_id, permission_slug; ")
        pu = None
        for (name, perm, description) in up:
            if pu != name:
                print(name)
                pu = name
            print("   ", perm, description)
    else:
        print("Unkown listing option", command)

