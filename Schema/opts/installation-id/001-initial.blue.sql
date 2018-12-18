INSERT INTO odin.module VALUES('opts/installation-id') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('opts/installation-id', '001-initial.blue.sql');

ALTER TABLE odin.identity  ADD COLUMN
    installation_id text NULL;

CREATE TABLE odin.identity_installation_id_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT installation_id_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_identity_installation_id_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    installation_id text NOT NULL,

    annotation json NOT NULL DEFAULT '{}'
);

CREATE FUNCTION odin.identity_installation_id_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.identity (id, installation_id)
            VALUES (NEW.identity_id, NEW.installation_id)
            ON CONFLICT (id) DO UPDATE SET
                installation_id = EXCLUDED.installation_id;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_identity_installation_id_ledger_insert_trigger
    AFTER INSERT ON odin.identity_installation_id_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.identity_installation_id_ledger_insert();

