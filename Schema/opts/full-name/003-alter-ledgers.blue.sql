INSERT INTO odin.migration VALUES('opts/full-name', '003-alter-ledgers.blue.sql');

ALTER TABLE odin.identity_full_name_ledger
    DROP CONSTRAINT credentials_password_ledger_identity_fkey,
    ADD CONSTRAINT credentials_password_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;
