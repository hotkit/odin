INSERT INTO odin.migration VALUES('opts/installation-id', '002-fix-pk.blue.sql');

ALTER TABLE odin.identity_installation_id_ledger
    DROP CONSTRAINT identity_installation_id_ledger_pkey,
    ADD PRIMARY KEY (reference);
