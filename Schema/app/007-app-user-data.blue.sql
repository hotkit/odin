CREATE TABLE odin.app_user_app_data_ledger (
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

    app_data JSON NOT NULL DEFAULT '{}',

    annotation json NOT NULL DEFAULT '{}'    
);

ALTER TABLE odin.app_user ADD COLUMN 
    app_data JSON NOT NULL DEFAULT '{}';

CREATE FUNCTION odin.app_user_app_data_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.app_user (app_id, identity_id, app_data)
            VALUES (NEW.app_id, NEW.identity_id, NEW.app_data)
            ON CONFLICT (app_id, identity_id) DO UPDATE SET
                app_data = EXCLUDED.app_data;
        RETURN NEW;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_user_app_data_ledger_insert_trigger
    BEFORE INSERT ON odin.app_user_app_data_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.app_user_app_data_ledger_insert();
