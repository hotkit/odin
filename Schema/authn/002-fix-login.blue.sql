INSERT INTO odin.module VALUES('authn') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('authn', '002-fix-login.blue.sql');

ALTER TABLE odin.login_success 
    DROP CONSTRAINT login_success_username_fkey;

ALTER TABLE odin.credentials 
    DROP CONSTRAINT credentials_login_pk,
    ADD CONSTRAINT credentials_login_pk PRIMARY KEY (identity_id),
    ADD CONSTRAINT login_uq UNIQUE (login);

ALTER TABLE odin.login_success
    ADD CONSTRAINT login_success_username_fkey
        FOREIGN KEY (username)
        REFERENCES odin.credentials (identity_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;

ALTER TABLE odin.credentials_password_ledger
  ADD COLUMN login TEXT NOT NULL;

DROP TRIGGER odin_credentials_ledger_insert_trigger ON odin.credentials_password_ledger;
DROP FUNCTION odin.credentials_ledger_insert();

CREATE FUNCTION odin.credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT
            INTO odin.credentials
                (identity_id, login, password__hash, password__process, password__reference)
            VALUES (NEW.identity_id, NEW.login, NEW.password, NEW.process, NEW.reference)
            ON CONFLICT (login) DO UPDATE SET
                password__hash = EXCLUDED.password__hash,
                password__process = EXCLUDED.password__process,
                password__reference = EXCLUDED.password__reference;
        IF NEW.hardens IS NOT NULL THEN
            UPDATE odin.credentials_password_ledger SET password=NULL
                WHERE identity_id=NEW.identity_id AND reference=NEW.hardens;
        END IF;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE TRIGGER odin_credentials_ledger_insert_trigger
    AFTER INSERT ON odin.credentials_password_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.credentials_ledger_insert();
