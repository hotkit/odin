INSERT INTO odin.module VALUES ('core');
INSERT INTO odin.migration VALUES('core', '001-initial.blue.sql');


CREATE FUNCTION odin.url_safe(str text) RETURNS boolean AS $body$
    BEGIN
        -- Disallow back slash, forward slash, fraction slash (2044),
        -- division slash (2215), reverse solidus operator (29f5),
        -- big solidus (29f8), big reverse solidus (29f9),
        -- small reverse solidus (fe68), fullwidth solidus (ff0f),
        -- full width reverse solidus (ff3c)
        RETURN str !~ '\\|/|\u2044|\u2215|\u29f5|\u29f8|\u29f9|\ufe68|\uff0f|\uff3c';
    END
$body$ LANGUAGE plpgsql;


CREATE TABLE odin.identity (
    id text NOT NULL CHECK (odin.url_safe(id)),
    CONSTRAINT identity_pk PRIMARY KEY (id),

    --- When the account expires
    expires timestamp with time zone NULL
);


CREATE TABLE odin.identity_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT odin_identity_ledger_pk PRIMARY KEY (reference, identity_id),

    created timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,
    annotation jsonb NOT NULL DEFAULT '{}'
);
CREATE FUNCTION identity_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT
            INTO odin.identity
            VALUES (NEW.identity_id)
            ON CONFLICT DO NOTHING;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER identity_ledger_insert_trigger
    AFTER INSERT ON odin.identity_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE identity_ledger_insert();


CREATE TABLE odin.identity_expiry_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT identity_expiry_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_identity_expiry_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    -- The time we want this account's password to expire
    expires timestamp with time zone NOT NULL,

    annotation jsonb NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.identity_expiry_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        UPDATE odin.identity SET
                expires = NEW.expires
            WHERE id=NEW.identity_id;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_identity_expiry_ledger_insert_trigger
    AFTER INSERT ON odin.identity_expiry_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.identity_expiry_ledger_insert();

