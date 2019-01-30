DROP TRIGGER odin_facebook_credentials_ledger_insert_trigger ON odin.facebook_credentials_ledger;

CREATE OR REPLACE FUNCTION odin.facebook_credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.facebook_credentials (identity_id, facebook_user_id)
            VALUES (NEW.identity_id, NEW.facebook_user_id)
            ON CONFLICT (identity_id, facebook_user_id) DO NOTHING;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_facebook_credentials_ledger_insert_trigger
    AFTER INSERT ON odin.facebook_credentials_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.facebook_credentials_ledger_insert();