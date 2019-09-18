INSERT INTO odin.migration
VALUES('app', '008-app-user-id.blue.sql');

CREATE TABLE odin.app_user_app_user_id_ledger (
    reference text NOT NULL,
    app_id text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT app_data_ledger_ledger_app_user_fkey
        FOREIGN KEY (app_id, identity_id)
        REFERENCES odin.app_user (app_id, identity_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_app_user_app_data_ledger_pk PRIMARY KEY (reference, app_id, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    app_user_id JSON NOT NULL,

    annotation json NOT NULL DEFAULT '{}'    
);

ALTER TABLE odin.app_user ADD COLUMN
    app_user_id TEXT NOT NULL DEFAULT '';

CREATE FUNCTION odin.app_user_app_user_id_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.app_user (app_id, identity_id, app_user_id)
            VALUES (NEW.app_id, NEW.identity_id, NEW.app_user_id)
            ON CONFLICT (app_id, identity_id) DO UPDATE SET
                app_user_id = EXCLUDED.app_user_id;
        RETURN NEW;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_user_app_user_id_ledger_insert_trigger
    BEFORE INSERT ON odin.app_user_app_user_id_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.app_user_app_user_id_ledger_insert();
