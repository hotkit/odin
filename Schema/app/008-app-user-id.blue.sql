
INSERT INTO odin.migration VALUES('app', '008-app-user-id.blue.sql');

/* Add column app_user_id to app_user and app_user_ledger*/
ALTER TABLE odin.app_user ADD COLUMN app_user_id TEXT;
UPDATE odin.app_user SET app_user_id = identity_id;
ALTER TABLE odin.app_user ALTER COLUMN app_user_id SET NOT NULL;

ALTER TABLE odin.app_user ADD CONSTRAINT app_user_app_user_id_unique UNIQUE (app_user_id, app_id);

ALTER TABLE odin.app_user_ledger ADD COLUMN app_user_id TEXT;
UPDATE odin.app_user_ledger SET app_user_id = identity_id;
ALTER TABLE odin.app_user_ledger ALTER COLUMN app_user_id SET NOT NULL;

DROP TRIGGER odin_app_user_ledger_insert_trigger ON odin.app_user_ledger;

CREATE OR REPLACE FUNCTION odin.app_user_ledger_insert() RETURNS TRIGGER AS $body$
BEGIN
    INSERT INTO odin.app_user (app_id, identity_id, app_user_id)
        VALUES (NEW.app_id, NEW.identity_id, NEW.app_user_id)
        ON CONFLICT (app_id, identity_id) DO NOTHING;
    RETURN NULL;
END;
$body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_user_ledger_insert_trigger
BEFORE INSERT ON odin.app_user_ledger
FOR EACH ROW
EXECUTE PROCEDURE odin.app_user_ledger_insert();

/* Change from INSERT ON CONFLICT FO UPDATE to UPDATE */
DROP TRIGGER odin_app_user_app_data_ledger_insert_trigger ON odin.app_user_app_data_ledger;

CREATE OR REPLACE FUNCTION odin.app_user_app_data_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        UPDATE odin.app_user SET app_data = NEW.app_data
            WHERE app_id = NEW.app_id AND identity_id = NEW.identity_id;
        IF NOT FOUND THEN RAISE EXCEPTION 'App user not found';
        END IF;
        RETURN NEW;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_user_app_data_ledger_insert_trigger
    BEFORE INSERT ON odin.app_user_app_data_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.app_user_app_data_ledger_insert();

/* Change from INSERT ON CONFLICT FO UPDATE to UPDATE */
DROP TRIGGER odin_app_user_installation_id_ledger_insert_trigger ON odin.app_user_installation_id_ledger;

CREATE OR REPLACE FUNCTION odin.app_user_installation_id_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        UPDATE odin.app_user SET installation_id = NEW.installation_id
            WHERE app_id = NEW.app_id AND identity_id = NEW.identity_id;
        IF NOT FOUND THEN RAISE EXCEPTION 'App user not found';
        END IF;
        RETURN NEW;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_user_installation_id_ledger_insert_trigger
    BEFORE INSERT ON odin.app_user_installation_id_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.app_user_installation_id_ledger_insert();

/* Change from reference to odin.app_user to odin.app and odin.identity */
ALTER TABLE odin.app_user_app_data_ledger DROP CONSTRAINT app_data_ledger_ledger_app_user_fkey;
ALTER TABLE odin.app_user_app_data_ledger
    ADD CONSTRAINT app_data_ledger_app_id_fkey
    FOREIGN KEY (app_id)
    REFERENCES odin.app (app_id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;
ALTER TABLE odin.app_user_app_data_ledger
    ADD CONSTRAINT app_data_ledger_identity_id_fkey
    FOREIGN KEY (identity_id)
    REFERENCES odin.identity_record (id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;

/* When merge two user with same app delete mrege_from user first for not break primary key*/
CREATE OR REPLACE FUNCTION odin.merge_account_app(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
    WITH from_app_user AS (
        SELECT * FROM odin.app_user
        WHERE identity_id=merge_from
    ), to_app_user AS (
        SELECT * FROM odin.app_user
        WHERE identity_id=merge_to
    ), duplicate_app_user AS (
        SELECT fa.app_id FROM from_app_user fa
        INNER JOIN to_app_user ta
        ON fa.app_id = ta.app_id
    ) DELETE FROM odin.app_user
    WHERE identity_id =  merge_from
    AND app_id IN (SELECT app_id FROM duplicate_app_user);

    UPDATE odin.app_user
    SET identity_id=merge_to
    WHERE identity_id=merge_from;
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
