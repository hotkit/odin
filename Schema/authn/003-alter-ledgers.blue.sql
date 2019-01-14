INSERT INTO odin.migration VALUES('authn', '003-alter-ledgers.blue.sql');


ALTER TABLE odin.credentials_password_ledger
    DROP CONSTRAINT credentials_password_ledger_identity_fkey,
    ADD CONSTRAINT credentials_password_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;

CREATE FUNCTION odin.merge_account_authn(merge_from TEXT, merge_to TEXT)
RETURNS BOOLEAN AS
$body$
BEGIN
    UPDATE odin.credentials
    SET identity_id=$merge_to
    WHERE identity_id=$merge_from;
    RETURN TRUE;
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
