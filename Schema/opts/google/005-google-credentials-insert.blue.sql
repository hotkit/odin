DROP FUNCTION odin.merge_account_opts_google(merge_from TEXT, merge_to TEXT);

DROP TRIGGER odin_google_credentials_ledger_insert_trigger ON odin.google_credentials_ledger;

CREATE OR REPLACE FUNCTION odin.google_credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.google_credentials (identity_id, google_user_id)
            VALUES (NEW.identity_id, NEW.google_user_id)
            ON CONFLICT (identity_id, google_user_id) DO NOTHING;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_google_credentials_ledger_insert_trigger
    AFTER INSERT ON odin.google_credentials_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.google_credentials_ledger_insert();