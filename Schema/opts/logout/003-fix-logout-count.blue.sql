INSERT INTO odin.migration VALUES('opts/logout', '003-fix-logout-count.blue.sql');


DROP TRIGGER odin_logout_ledger_insert_trigger ON odin.logout_ledger;
DROP FUNCTION odin.logout_ledger_insert();

CREATE FUNCTION odin.logout_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        UPDATE odin.credentials SET
                logout_count = logout_count + 1
            WHERE identity_id = NEW.identity_id;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_logout_ledger_insert_trigger
    AFTER INSERT ON odin.logout_ledger
    FOR EACH ROW EXECUTE PROCEDURE odin.logout_ledger_insert();
 
 