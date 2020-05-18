INSERT INTO odin.migration VALUES('opts/logout', '004-alter-ledgers.blue.sql');

ALTER TABLE odin.logout_ledger
    DROP CONSTRAINT logout_ledger_identity_fkey,
    ADD CONSTRAINT logout_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE
