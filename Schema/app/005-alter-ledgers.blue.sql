INSERT INTO odin.migration VALUES('app', '005-alter-ledgers.blue.sql');

ALTER TABLE odin.app_user_ledger
    DROP CONSTRAINT app_user_ledger_app_id_fkey,
    ADD CONSTRAINT app_user_ledger_app_id_fkey
        FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    DROP CONSTRAINT app_user_ledger_identity_id_fkey,
    ADD CONSTRAINT app_user_ledger_identity_id_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;
