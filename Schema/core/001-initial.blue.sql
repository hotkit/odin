INSERT INTO odin.module VALUES ('core');
INSERT INTO odin.migration VALUES('core', '001-initial.blue.sql');


CREATE TABLE odin.identity (
    id text NOT NULL,
    CONSTRAINT identity_pk PRIMARY KEY (id)
);


CREATE TABLE odin.identity_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT odin_identity_ledger_pk PRIMARY KEY (reference, identity_id),

    created timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,
    annotation jsonb NOT NULL DEFAULT '{}'
);
CREATE FUNCTION identity_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.identity VALUES (NEW.identity_id);
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER identity_ledger_insert_trigger
    AFTER INSERT ON odin.identity_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE identity_ledger_insert();

