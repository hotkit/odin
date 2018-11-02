INSERT INTO odin.module VALUES('app') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('app', '002-initial.blue.sql');


-- APP
CREATE TABLE odin.app (
    app_id TEXT NOT NULL,
    PRIMARY KEY (app_id),
    FOREIGN KEY (app_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,

    app_name TEXT NOT NULL,
    token TEXT NOT NULL,
    access_policy TEXT NOT NULL,
    data_sharing_policy TEXT NOT NULL,
    redirect_url TEXT NOT NULL
);

CREATE TABLE odin.app_ledger (
    reference TEXT NOT NULL,
    app_id TEXT NOT NULL,
    PRIMARY KEY (reference, app_id),

    app_name TEXT NOT NULL,
    token TEXT NOT NULL,
    access_policy TEXT NOT NULL DEFAULT 'OPEN',
    data_sharing_policy TEXT NOT NULL DEFAULT 'ALL',
    redirect_url TEXT NOT NULL,

    changed TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    pg_user TEXT NOT NULL DEFAULT current_user,
    annotation JSON NOT NULL DEFAULT '{}'
);

CREATE FUNCTION odin.app_ledger_insert() RETURNS TRIGGER AS $body$
BEGIN
    INSERT INTO odin.app (app_id, app_name, token, access_policy, data_sharing_policy, redirect_url)
    VALUES (NEW.app_id, NEW.app_name, NEW.token, NEW.access_policy, NEW.data_sharing_policy, NEW.redirect_url)
    ON CONFLICT (app_id) DO UPDATE SET
        app_name = EXCLUDED.app_name,
        access_policy = EXCLUDED.access_policy,
        data_sharing_policy=NEW.data_sharing_policy,
        redirect_url=NEW.redirect_url,
        token=NEW.token;
    RETURN NULL;
END;
$body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_ledger_insert_trigger
AFTER INSERT ON odin.app_ledger
FOR EACH ROW
EXECUTE PROCEDURE odin.app_ledger_insert();

-- APP USER
CREATE TABLE odin.app_user (
    app_id TEXT NOT NULL,
    FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    identity_id TEXT NOT NULL,
    FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,

    PRIMARY KEY (app_id, identity_id),

    state TEXT NOT NULL
);

CREATE TABLE odin.app_user_ledger (
    reference TEXT NOT NULL,
    app_id TEXT NOT NULL,
    FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    identity_id TEXT NOT NULL,
    FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,

    PRIMARY KEY (reference, app_id, identity_id),

    state TEXT NOT NULL DEFAULT 'ACTIVE',
    changed TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    pg_user TEXT NOT NULL DEFAULT current_user,
    annotation JSON NOT NULL DEFAULT '{}'
);

CREATE FUNCTION odin.app_user_ledger_insert() RETURNS TRIGGER AS $body$
BEGIN
    INSERT INTO odin.app_user (app_id, identity_id, state)
    VALUES (NEW.app_id, NEW.identity_id, NEW.state)
    ON CONFLICT (app_id, identity_id) DO UPDATE SET
        state=EXCLUDED.state;
    RETURN NULL;
END;
$body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_user_ledger_insert_trigger
AFTER INSERT ON odin.app_user_ledger
FOR EACH ROW
EXECUTE PROCEDURE odin.app_user_ledger_insert();
