INSERT INTO odin.migration VALUES('authz', '003-alter-ledgers.blue.sql');


ALTER TABLE odin.identity_superuser_ledger
    DROP CONSTRAINT credentials_superuser_ledger_identity_fkey,
    ADD CONSTRAINT credentials_superuser_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;

ALTER TABLE odin.group_membership_ledger
    DROP CONSTRAINT odin_group_membership_ledger_identity_fkey,
    ADD CONSTRAINT odin_group_membership_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;
