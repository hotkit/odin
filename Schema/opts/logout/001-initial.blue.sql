INSERT INTO odin.migration VALUES('opts/logout', '001-initial.blue.sql');


ALTER TABLE odin.credentials ADD COLUMN
    logout_count int NOT NULL DEFAULT 0;


CREATE TABLE odin.logout_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT logout_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_logout_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    source_address text NULL,
    annotation json NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.logout_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        UPDATE odin.credentials SET
            logout_count = logout_count + 1;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_logout_ledger_insert_trigger
    AFTER INSERT ON odin.logout_ledger
    FOR EACH ROW EXECUTE PROCEDURE odin.logout_ledger_insert();

