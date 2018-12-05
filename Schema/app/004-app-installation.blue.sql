INSERT INTO odin.migration VALUES('app', '004-app-installation.blue.sql');

ALTER TABLE odin.app_user ADD COLUMN
    installation_id text NULL;

CREATE TABLE odin.app_user_installation_id_ledger (
    reference text NOT NULL,
    app_id text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT installation_id_ledger_app_user_fkey
        FOREIGN KEY (app_id, identity_id)
        REFERENCES odin.app_user (app_id, identity_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_app_user_installation_id_ledger_pk PRIMARY KEY (reference, app_id, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    installation_id text NOT NULL,

    annotation json NOT NULL DEFAULT '{}'
);

CREATE FUNCTION odin.app_user_installation_id_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.app_user (app_id, identity_id, installation_id)
            VALUES (NEW.app_id, NEW.identity_id, NEW.installation_id)
            ON CONFLICT (app_id, identity_id) DO UPDATE SET
                installation_id = EXCLUDED.installation_id;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_app_user_installation_id_ledger_insert_trigger
    AFTER INSERT ON odin.app_user_installation_id_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.app_user_installation_id_ledger_insert();
