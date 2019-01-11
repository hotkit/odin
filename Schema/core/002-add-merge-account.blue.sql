INSERT INTO odin.migration VALUES('core', '002-add-merge-account.blue.sql');

CREATE TABLE odin.identity_record(
    id text NOT NULL CHECK (odin.url_safe(id)),
    PRIMARY KEY (id)
);

ALTER TABLE odin.identity
    ADD CONSTRAINT identity_id_fkey FOREIGN KEY (id) REFERENCES odin.identity_record(id);

CREATE FUNCTION identity_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT
            INTO odin.identity_record
            VALUES (NEW.id)
            ON CONFLICT DO NOTHING;
        RETURN NEW;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER identity_insert_trigger
    BEFORE INSERT ON odin.identity
    FOR EACH ROW
    EXECUTE PROCEDURE identity_insert();

CREATE TABLE odin.merge_record(
    from_identity_id TEXT NOT NULL,
    FOREIGN KEY (from_identity_id) REFERENCES odin.identity_record(id),
    to_identity_id TEXT NOT NULL,
    FOREIGN KEY (to_identity_id) REFERENCES odin.identity_record(id),
    PRIMARY KEY (from_identity_id, to_identity_id),
    merged TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);
