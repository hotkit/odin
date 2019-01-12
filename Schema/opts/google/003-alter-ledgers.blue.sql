INSERT INTO odin.migration VALUES('opts/google', '003-alter-ledgers.blue.sql');

ALTER TABLE odin.google_credentials_ledger
    DROP CONSTRAINT google_credentials_ledger_identity_fkey,
    ADD CONSTRAINT google_credentials_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;
