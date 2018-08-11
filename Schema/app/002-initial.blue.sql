INSERT INTO odin.module VALUES('app') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('app', '002-initial.blue.sql');


-- APP
CREATE TABLE odin.app (
    app_id TEXT NOT NULL CHECK (odin.url_safe(app_id)),
    app_name TEXT NOT NULL,
    access_policy TEXT NOT NULL,
    data_sharing_policy TEXT NOT NULL,
    redirect_url TEXT NOT NULL,
    changed TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    created TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    PRIMARY KEY (app_id)
);

CREATE TABLE odin.app_ledger (
    reference TEXT NOT NULL,
    app_id TEXT NOT NULL,
    app_name TEXT NOT NULL,
    access_policy TEXT NOT NULL DEFAULT 'OPEN',
    data_sharing_policy TEXT NOT NULL DEFAULT 'ALL',
    redirect_url TEXT NOT NULL,
    changed TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    pg_user TEXT NOT NULL DEFAULT current_user,
    annotation JSON NOT NULL DEFAULT '{}',
    PRIMARY KEY (reference, app_id)
);

CREATE FUNCTION odin.app_ledger_insert() RETURNS TRIGGER AS $body$
BEGIN
    INSERT INTO odin.app (app_id, app_name, access_policy, data_sharing_policy, redirect_url, changed, created)
    VALUES (NEW.app_id, NEW.app_name, NEW.access_policy, NEW.data_sharing_policy, NEW.redirect_url, NEW.changed, NEW.changed)
    ON CONFLICT (app_id) DO UPDATE SET
        app_name = EXCLUDED.app_name,
        access_policy = EXCLUDED.access_policy,
        data_sharing_policy=NEW.data_sharing_policy,
        redirect_url=NEW.redirect_url,
        changed = EXCLUDED.changed;
    RETURN NULL;
END;
$body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_ledger_insert_trigger
AFTER INSERT ON odin.app_ledger
FOR EACH ROW
EXECUTE PROCEDURE odin.app_ledger_insert();

-- APP OWNER
CREATE TABLE odin.app_owner (
    identity_id TEXT NOT NULL,
    app_id TEXT NOT NULL,
    state TEXT NOT NULL,
    changed TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    pg_user TEXT NOT NULL,
    PRIMARY KEY (identity_id, app_id),
    FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE
);

CREATE TABLE odin.app_owner_ledger (
    reference TEXT NOT NULL,
    identity_id TEXT NOT NULL,
    app_id TEXT NOT NULL,
    state TEXT NOT NULL DEFAULT 'ACTIVE',
    changed TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    pg_user TEXT NOT NULL DEFAULT current_user,
    PRIMARY KEY (reference, identity_id, app_id),
    FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE
);

CREATE FUNCTION odin.app_owner_ledger_insert() RETURNS TRIGGER AS $body$
BEGIN
    INSERT INTO odin.app_owner (identity_id, app_id, state, changed, pg_user)
    VALUES (NEW.identity_id, NEW.app_id, NEW.state, NEW.changed, NEW.pg_user)
    ON CONFLICT (identity_id, app_id) DO UPDATE SET
        state=EXCLUDED.state,
        changed=EXCLUDED.changed,
        pg_user=EXCLUDED.pg_user;
    RETURN NULL;
END;
$body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_owner_ledger_insert_trigger
AFTER INSERT ON odin.app_owner_ledger
FOR EACH ROW
EXECUTE PROCEDURE odin.app_owner_ledger_insert();

-- APP USER
CREATE TABLE odin.app_user (
    identity_id TEXT NOT NULL,
    app_id TEXT NOT NULL,
    state TEXT NOT NULL,
    changed TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    pg_user TEXT NOT NULL,
    PRIMARY KEY (identity_id, app_id),
    FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE
);

CREATE TABLE odin.app_user_ledger (
    reference TEXT NOT NULL,
    identity_id TEXT NOT NULL,
    app_id TEXT NOT NULL,
    state TEXT NOT NULL DEFAULT 'ACTIVE',
    changed TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    pg_user TEXT NOT NULL DEFAULT current_user,
    PRIMARY KEY (reference, identity_id, app_id),
    FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE
);

CREATE FUNCTION odin.app_user_ledger_insert() RETURNS TRIGGER AS $body$
BEGIN
    INSERT INTO odin.app_user (identity_id, app_id, state, changed, pg_user)
    VALUES (NEW.identity_id, NEW.app_id, NEW.state, NEW.changed, NEW.pg_user)
    ON CONFLICT (identity_id, app_id) DO UPDATE SET
        state=EXCLUDED.state,
        changed=EXCLUDED.changed,
        pg_user=EXCLUDED.pg_user;
    RETURN NULL;
END;
$body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_user_ledger_insert_trigger
AFTER INSERT ON odin.app_user_ledger
FOR EACH ROW
EXECUTE PROCEDURE odin.app_user_ledger_insert();
