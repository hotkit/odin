INSERT INTO odin.migration VALUES('opts/facebook', '004-merge-account-function.blue.sql');

DROP TRIGGER odin_facebook_credentials_ledger_insert_trigger ON odin.facebook_credentials_ledger;

CREATE OR REPLACE FUNCTION odin.facebook_credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.facebook_credentials (identity_id, facebook_user_id)
            VALUES (NEW.identity_id, NEW.facebook_user_id);
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_facebook_credentials_ledger_insert_trigger
    AFTER INSERT ON odin.facebook_credentials_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.facebook_credentials_ledger_insert();

ALTER TABLE odin.facebook_credentials
    DROP CONSTRAINT facebook_credentials_pk,
    ADD CONSTRAINT facebook_credentials_pk
        PRIMARY KEY (identity_id, facebook_user_id);

CREATE FUNCTION odin."merge_account_opts/facebook"(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
    UPDATE odin.facebook_credentials
    SET identity_id=merge_to
    WHERE identity_id=merge_from;    
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
