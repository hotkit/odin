INSERT INTO odin.migration VALUES('opts/installation-id', '003-alter-ledgers.blue.sql');

ALTER TABLE odin.identity_installation_id_ledger
    DROP CONSTRAINT installation_id_ledger_identity_fkey,
    ADD CONSTRAINT installation_id_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;
