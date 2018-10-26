INSERT INTO odin.migration VALUES('app', '003-app-role.blue.sql');

-- APP ROLE
CREATE TABLE odin.app_role (
    app_id TEXT NOT NULL,
    FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    role TEXT NOT NULL,
    PRIMARY KEY (app_id, role),
    changed TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    pg_user TEXT NOT NULL
);

-- APP USER ROLE
CREATE TABLE odin.app_user_role (
    identity_id TEXT NOT NULL,
    app_id TEXT NOT NULL,
    FOREIGN KEY (identity_id, app_id)
        REFERENCES odin.app_user (identity_id, app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    role TEXT NOT NULL,
    FOREIGN KEY (app_id, role)
        REFERENCES odin.app_role (app_id, role) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    PRIMARY KEY (app_id, identity_id, role),
    changed TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    pg_user TEXT NOT NULL
);
