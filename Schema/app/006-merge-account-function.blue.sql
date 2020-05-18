INSERT INTO odin.migration VALUES('app', '006-merge-account-function.blue.sql');

ALTER TABLE odin.app_user_role_ledger
    DROP CONSTRAINT app_user_role_ledger_app_id_fkey,
    ADD CONSTRAINT app_user_role_ledger_app_id_fkey
        FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    ADD CONSTRAINT app_user_role_ledger_identity_id_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;

ALTER TABLE odin.app_user_role
    DROP CONSTRAINT app_user_role_app_id_fkey,
    ADD CONSTRAINT app_user_role_app_id_fkey
    FOREIGN KEY (app_id, identity_id)
        REFERENCES odin.app_user (app_id, identity_id) MATCH SIMPLE
        ON UPDATE CASCADE ON DELETE NO ACTION DEFERRABLE;

CREATE FUNCTION odin.merge_account_app(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
    UPDATE odin.app_user
    SET identity_id=merge_to
    WHERE identity_id=merge_from;
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
