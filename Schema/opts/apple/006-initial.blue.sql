INSERT INTO odin.module VALUES('opts/apple') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('opts/apple', '006-initial.blue.sql');

CREATE TABLE odin.apple_credentials (
    identity_id text NOT NULL,
    CONSTRAINT apple_credentials_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    apple_user_id text NOT NULL UNIQUE,
    CONSTRAINT apple_credentials_pk PRIMARY KEY (identity_id, apple_user_id)
);


CREATE TABLE odin.apple_credentials_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT apple_credentials_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_apple_credentials_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,
    apple_user_id text NOT NULL,
    annotation json NOT NULL DEFAULT '{}'
);

CREATE FUNCTION odin.apple_credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.apple_credentials (identity_id, apple_user_id)
            VALUES (NEW.identity_id, NEW.apple_user_id)
            ON CONFLICT (identity_id, apple_user_id) DO NOTHING;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_apple_credentials_ledger_insert_trigger
    AFTER INSERT ON odin.apple_credentials_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.apple_credentials_ledger_insert();

CREATE FUNCTION odin."merge_account_opts/apple"(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

