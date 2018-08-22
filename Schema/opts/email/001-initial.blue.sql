INSERT INTO odin.module VALUES('opts/email') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('opts/email', '001-initial.blue.sql');


ALTER TABLE odin.identity  ADD COLUMN
    email text NULL UNIQUE;

CREATE TABLE odin.identity_email_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT credentials_password_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_identity_email_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    email text NOT NULL,

    annotation json NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.identity_email_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.identity (id, email)
            VALUES (NEW.identity_id, NEW.email)
            ON CONFLICT (id) DO UPDATE SET
                email = EXCLUDED.email;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_identity_email_ledger_insert_trigger
    AFTER INSERT ON odin.identity_email_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.identity_email_ledger_insert();

