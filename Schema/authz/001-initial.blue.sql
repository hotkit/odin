INSERT INTO odin.module VALUES('authz');
INSERT INTO odin.migration VALUES('authz', '001-initial.blue.sql');


ALTER TABLE odin.identity ADD COLUMN
    is_superuser boolean NOT NULL DEFAULT 'f';

CREATE TABLE odin.identity_superuser_ledger (
    identity_id text NOT NULL,
    CONSTRAINT credentials_superuser_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    changed timestamp with time zone NOT NULL DEFAULT now(),
    CONSTRAINT odin_identity_superuser_ledger_pk PRIMARY KEY (identity_id, changed),

    superuser boolean NOT NULL,
    annotation json NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.identity_superuser_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.identity (id, superuser)
            VALUES (NEW.identity_id, is_superuser)
            ON CONFLICT (id) DO UPDATE SET
                is_superuser = EXCLUDED.superuser;
        RETURN NULL;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_identity_superuser_ledger_insert_trigger
    AFTER INSERT ON odin.identity_superuser_ledger
    FOR EACH ROW EXECUTE PROCEDURE odin.identity_superuser_ledger_insert();


CREATE TABLE odin.group (
    slug text NOT NULL,
    CONSTRAINT odin_group_pk PRIMARY KEY (slug),

    description text NOT NULL DEFAULT ''
);


CREATE TABLE odin.group_membership (
    identity_id text NOT NULL,
    CONSTRAINT odin_group_membership_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    group_slug text NOT NULL,
    CONSTRAINT odin_group_membership_group_fkey
        FOREIGN KEY (group_slug)
        REFERENCES odin.group (slug) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_group_membership_pk PRIMARY KEY (identity_id, group_slug)
);


CREATE TABLE odin.permission (
    slug text NOT NULL,
    CONSTRAINT odin_permission_pk PRIMARY KEY (slug),

    description text NOT NULL DEFAULT ''
);


CREATE TABLE odin.group_grant (
    group_slug text NOT NULL,
    CONSTRAINT odin_group_grant_group_fkey
        FOREIGN KEY (group_slug)
        REFERENCES odin.group (slug) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    permission_slug text NOT NULL,
    CONSTRAINT odin_group_grant_pemmission_fkey
        FOREIGN KEY (permission_slug)
        REFERENCES odin.permission (slug) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_group_grant_pk PRIMARY KEY (group_slug, permission_slug)
);

