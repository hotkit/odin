INSERT INTO odin.migration VALUES('opts/facebook', '003-alter-ledgers.blue.sql');

ALTER TABLE odin.facebook_credentials_ledger
    DROP CONSTRAINT facebook_credentials_ledger_identity_fkey,
    ADD CONSTRAINT facebook_credentials_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;
