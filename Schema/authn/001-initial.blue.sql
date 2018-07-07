INSERT INTO odin.migration VALUES('authn', '001-initial.blue.sql');


CREATE TABLE odin.credentials (
    identity_id text NOT NULL,
    CONSTRAINT credentials_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,

    -- Login name, the name used by the user in the log in form
    login text NOT NULL,
    CONSTRAINT credentials_login_pk PRIMARY KEY (login),

    -- The password/hash value
    password__hash text NULL,
    -- The process for generating the password hash
    password__process jsonb NULL,
    --- When the password was last set
    password__reference text NULL
);

CREATE TABLE odin.credentials_password_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT credentials_password_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_credentials_password_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    -- The password/hash value
    password text NULL,
    -- The process for generating the password hash
    process jsonb NOT NULL,

    -- Set this if the new hash is a hardened old hash (which must be cleared)
    hardens text NULL,
    CONSTRAINT odin_credentials_ledger_hardens_fkey
        FOREIGN KEY (hardens, identity_id)
        REFERENCES odin.credentials_password_ledger (reference, identity_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,

    annotation jsonb NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT
            INTO odin.credentials
                (identity_id, login, password__hash, password__process, password__reference)
            VALUES (NEW.identity_id, NEW.identity_id, NEW.password, NEW.process, NEW.reference)
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


CREATE TABLE odin.login_attempt (
    username text NOT NULL,
    attempt timestamp with time zone NOT NULL DEFAULT now(),

    source_address text NULL,
    annotation jsonb NOT NULL DEFAULT '{}'
);

CREATE TABLE odin.login_failed (
    CONSTRAINT login_attempt_pt PRIMARY KEY (username, attempt)
) INHERITS (odin.login_attempt);
CREATE INDEX login_attempt_failed_source_ix
    ON odin.login_failed (source_address, attempt)
    WHERE source_address IS NOT NULL;

CREATE TABLE odin.login_success (
    CONSTRAINT login_success_username_fkey
        FOREIGN KEY (username)
        REFERENCES odin.credentials (login) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE
) INHERITS (odin.login_attempt);
CREATE INDEX login_success_source_ix
    ON odin.login_attempt (source_address, attempt)
    WHERE source_address IS NOT NULL;

