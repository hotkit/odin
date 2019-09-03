ALTER TABLE odin.app_user_ledger ADD COLUMN app_data JSON NOT NULL DEFAULT '{}';
ALTER TABLE odin.app_user ADD COLUMN app_data JSON NOT NULL DEFAULT '{}';

CREATE OR REPLACE FUNCTION odin.app_user_ledger_insert() RETURNS TRIGGER AS $body$
BEGIN
    INSERT INTO odin.app_user (app_id, identity_id, app_data)
    VALUES (NEW.app_id, NEW.identity_id, NEW.app_data)
    ON CONFLICT (app_id, identity_id) DO NOTHING;
    RETURN NULL;
END;
$body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

DROP TRIGGER odin_app_user_ledger_insert_trigger ON odin.app_user_ledger; 
CREATE TRIGGER odin_app_user_ledger_insert_trigger
AFTER INSERT ON odin.app_user_ledger
FOR EACH ROW
EXECUTE PROCEDURE odin.app_user_ledger_insert();