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

CREATE FUNCTION odin.merge_account()
RETURNS TRIGGER AS $body$
DECLARE
    mods RECORD;
BEGIN
    FOR mods IN
        SELECT REPLACE(name, '/', '_') AS name
        FROM odin.module
        WHERE name != 'core'
    LOOP
        EXECUTE FORMAT('SELECT odin.merge_account_%I($1, $2)', mods.name)
            USING NEW.from_identity_id, NEW.to_identity_id;
    END LOOP;
    EXECUTE 'DELETE FROM odin.identity WHERE id=$1' USING NEW.from_identity_id;
    RETURN NEW;
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER insert_merge_record
    AFTER INSERT ON odin.merge_record
    FOR EACH ROW
    EXECUTE PROCEDURE odin.merge_account();
