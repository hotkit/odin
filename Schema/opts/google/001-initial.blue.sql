INSERT INTO odin.module VALUES('opts/google') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('opts/google', '001-initial.blue.sql');


CREATE TABLE odin.google_credentials (
    identity_id text NOT NULL,
    CONSTRAINT google_credentials_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    google_user_id text NOT NULL UNIQUE,
    CONSTRAINT google_credentials_pk PRIMARY KEY (identity_id)
);


CREATE TABLE odin.google_credentials_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT google_credentials_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_google_credentials_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,
    google_user_id text NOT NULL,
    annotation json NOT NULL DEFAULT '{}'
);

CREATE FUNCTION odin.google_credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.google_credentials (identity_id, google_user_id)
            VALUES (NEW.identity_id, NEW.google_user_id)
            ON CONFLICT (identity_id) DO UPDATE SET
                google_user_id = EXCLUDED.google_user_id;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_google_credentials_ledger_insert_trigger
    AFTER INSERT ON odin.google_credentials_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.google_credentials_ledger_insert();

