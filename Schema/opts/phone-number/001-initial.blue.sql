INSERT INTO odin.module VALUES('opts/phone-number') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('opts/phone-number', '001-initial.blue.sql');


ALTER TABLE odin.identity  ADD COLUMN
    phone_number text NULL;

CREATE TABLE odin.identity_phone_number_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT odin_identity_phone_number_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_identity_phone_number_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    phone_number text NOT NULL,

    annotation json NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.identity_phone_number_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.identity (id, phone_number)
            VALUES (NEW.identity_id, NEW.phone_number)
            ON CONFLICT (id) DO UPDATE SET
                phone_number = EXCLUDED.phone_number;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_identity_phone_number_ledger_insert_trigger
    AFTER INSERT ON odin.identity_phone_number_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.identity_phone_number_ledger_insert();

