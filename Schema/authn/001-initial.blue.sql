

CREATE TABLE odin.credentials (
    identity_id text NOT NULL,
    CONSTRAINT credentials_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT credentials_pk PRIMARY KEY (identity_id),

    -- Login name, the name used by the user in the log in form
    login text NOT NULL,
    CONSTRAINT credentials_login_ix UNIQUE (login),
    -- The password/hash value
    password__hash text NULL,
    -- The process for generating the password hash
    password__process jsonb NULL,
    --- When the password was last set
    password__changed timestamp with time zone NULL
);


CREATE TABLE odin.credentials_password_ledger (
    identity_id text NOT NULL,
    CONSTRAINT credentials_password_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    changed timestamp with time zone NOT NULL DEFAULT now(),
    CONSTRAINT password_pk PRIMARY KEY (identity_id, changed),

    -- The password/hash value
    password text NULL,
    -- The process for generating the password hash
    process jsonb NOT NULL,

    -- Set this if the new hash is a hardened old hash (which must be cleared)
    hardens timestamp with time zone NULL,
    CONSTRAINT credentials_ledger_hardens_fkey
        FOREIGN KEY (identity_id, hardens)
        REFERENCES odin.credentials_password_ledger (identity_id, changed) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,

    annotation jsonb NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.credentials_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT
            INTO odin.credentials (identity_id, login, password__hash,
                password__process, password__changed)
            VALUES (NEW.identity_id, NEW.identity_id, NEW.password, NEW.process, NEW.changed)
            ON CONFLICT (identity_id) DO UPDATE
                SET
                    password__hash = EXCLUDED.password__hash,
                    password__process = EXCLUDED.password__process,
                    password__changed = EXCLUDED.password__changed;
        IF NEW.hardens IS NOT NULL THEN
            UPDATE odin.credentials_password_ledger SET password=NULL
                WHERE identity_id=NEW.identity_id AND changed=NEW.hardens;
        END IF;
        RETURN NULL;
    END;
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER credentials_ledger_trigger
    AFTER INSERT ON odin.credentials_password_ledger
    FOR EACH ROW
    EXECUTE PROCEDURE odin.credentials_ledger_insert();

