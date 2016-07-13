CREATE SCHEMA odin;


CREATE TABLE identity (
    id text NOT NULL,
    CONSTRAINT identity_pk PRIMARY KEY (id)
);


CREATE TABLE identity_ledger (
    identity_id text NOT NULL,
    CONSTRAINT odint_identity_ledger_pk PRIMARY KEY (identity_id),
    created timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,
    annotation jsonb NOT NULL DEFAULT '{}'
);
CREATE FUNCTION identity_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO identity VALUES (NEW.identity_id);
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER identity_ledger_insert_trigger
    AFTER INSERT ON identity_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE identity_ledger_insert();

INSERT INTO identity_ledger VALUES (current_user);

