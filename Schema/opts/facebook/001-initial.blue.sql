INSERT INTO odin.module VALUES('opts/facebook') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('opts/facebook', '001-initial.blue.sql');


CREATE TABLE odin.facebook_credentials (
    identity_id text NOT NULL,
    CONSTRAINT facebook_credentials_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    facebook_user_id text NOT NULL UNIQUE,
    CONSTRAINT facebook_credentials_pk PRIMARY KEY (identity_id)
);


CREATE TABLE odin.facebook_credentials_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT facebook_credentials_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_facebook_credentials_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,
    facebook_user_id text NOT NULL,
    annotation json NOT NULL DEFAULT '{}'
);

CREATE FUNCTION odin.facebook_credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.facebook_credentials (identity_id, facebook_user_id)
            VALUES (NEW.identity_id, NEW.facebook_user_id)
            ON CONFLICT (identity_id) DO UPDATE SET
                facebook_user_id = EXCLUDED.facebook_user_id;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_facebook_credentials_ledger_insert_trigger
    AFTER INSERT ON odin.facebook_credentials_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.facebook_credentials_ledger_insert();

