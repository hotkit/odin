INSERT INTO odin.module VALUES('opts/full-name') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('opts/full-name', '001-initial.blue.sql');


ALTER TABLE odin.identity  ADD COLUMN
    full_name text NULL;

CREATE TABLE odin.identity_full_name_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT credentials_password_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_identity_full_name_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    full_name text NOT NULL,

    annotation json NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.identity_full_name_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.identity (id, full_name)
            VALUES (NEW.identity_id, NEW.full_name)
            ON CONFLICT (id) DO UPDATE SET
                full_name = EXCLUDED.full_name;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_identity_full_name_ledger_insert_trigger
    AFTER INSERT ON odin.identity_full_name_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.identity_full_name_ledger_insert();

