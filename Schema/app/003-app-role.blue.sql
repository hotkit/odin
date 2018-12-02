INSERT INTO odin.migration VALUES('app', '003-app-role.blue.sql');

-- APP ROLE
CREATE TABLE odin.app_role (
    app_id TEXT NOT NULL,
    FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    role TEXT NOT NULL CHECK (odin.url_safe(role)),
    PRIMARY KEY (app_id, role)
);


CREATE TABLE odin.app_role_ledger (
    reference text NOT NULL,
    app_id text NOT NULL,
    FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE,
    role TEXT NOT NULL CHECK (odin.url_safe(role)),
    CONSTRAINT odin_app_role_ledger_pk PRIMARY KEY (reference, app_id, role),

    created timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,
    annotation jsonb NOT NULL DEFAULT '{}'
);


CREATE FUNCTION app_role_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT
            INTO odin.app_role (app_id, role)
            VALUES (NEW.app_id, NEW.role)
            ON CONFLICT DO NOTHING;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;


CREATE TRIGGER app_role_ledger_insert_trigger
    AFTER INSERT ON odin.app_role_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE app_role_ledger_insert();


-- APP USER ROLE
CREATE TABLE odin.app_user_role (
    app_id TEXT NOT NULL,
    identity_id TEXT NOT NULL,
    FOREIGN KEY (app_id, identity_id)
        REFERENCES odin.app_user (app_id, identity_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    role TEXT NOT NULL,
    FOREIGN KEY (app_id, role)
        REFERENCES odin.app_role (app_id, role) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    PRIMARY KEY (app_id, identity_id, role)
);

CREATE TABLE odin.app_user_role_ledger (
    reference text NOT NULL,
    app_id text NOT NULL,
    identity_id TEXT NOT NULL,
    FOREIGN KEY (app_id, identity_id)
        REFERENCES odin.app_user (app_id, identity_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    role TEXT NOT NULL,
    FOREIGN KEY (app_id, role)
        REFERENCES odin.app_role (app_id, role) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_app_user_role_ledger_pk PRIMARY KEY (reference, app_id, identity_id, role),

    created timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,
    annotation jsonb NOT NULL DEFAULT '{}'
);


CREATE FUNCTION app_user_role_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT
            INTO odin.app_user_role (app_id, identity_id, role)
            VALUES (NEW.app_id, NEW.identity_id, NEW.role)
            ON CONFLICT DO NOTHING;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;


CREATE TRIGGER app_user_role_ledger_insert_trigger
    AFTER INSERT ON odin.app_user_role_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE app_user_role_ledger_insert();
