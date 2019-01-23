INSERT INTO odin.migration VALUES('opts/google', '004-merge-account-function.blue.sql');

DROP TRIGGER odin_google_credentials_ledger_insert_trigger ON odin.google_credentials_ledger;

CREATE OR REPLACE FUNCTION odin.google_credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.google_credentials (identity_id, google_user_id)
            VALUES (NEW.identity_id, NEW.google_user_id);
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_google_credentials_ledger_insert_trigger
    AFTER INSERT ON odin.google_credentials_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.google_credentials_ledger_insert();

ALTER TABLE odin.google_credentials
    DROP CONSTRAINT google_credentials_identity_fkey,
    DROP CONSTRAINT google_credentials_pk,
    ADD CONSTRAINT google_credentials_pk
        PRIMARY KEY (identity_id, google_user_id);

CREATE FUNCTION odin.merge_account_opts_google(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
    UPDATE odin.google_credentials
    SET identity_id=merge_to
    WHERE identity_id=merge_from;    
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;



CREATE FUNCTION odin."merge_account_opts/google"(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
    UPDATE odin.google_credentials
    SET identity_id=merge_to
    WHERE identity_id=merge_from;    
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
