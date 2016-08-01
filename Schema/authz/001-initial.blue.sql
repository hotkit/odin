INSERT INTO odin.module VALUES('authz');
INSERT INTO odin.migration VALUES('authz', '001-initial.blue.sql');


-- Super users will always be allowed every permission
ALTER TABLE odin.identity ADD COLUMN
    is_superuser boolean NOT NULL DEFAULT 'f';

CREATE TABLE odin.identity_superuser_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT credentials_superuser_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_identity_superuser_ledger_pk PRIMARY KEY (reference, identity_id),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    superuser boolean NOT NULL,

    annotation json NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.identity_superuser_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.identity (id, is_superuser)
            VALUES (NEW.identity_id, NEW.superuser)
            ON CONFLICT (id) DO UPDATE SET
                is_superuser = EXCLUDED.is_superuser;
        RETURN NULL;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_identity_superuser_ledger_insert_trigger
    AFTER INSERT ON odin.identity_superuser_ledger
    FOR EACH ROW EXECUTE PROCEDURE odin.identity_superuser_ledger_insert();


-- Groups are used to manage users and permissions
CREATE TABLE odin.group (
    slug text NOT NULL CHECK (odin.url_safe(slug)),
    CONSTRAINT odin_group_pk PRIMARY KEY (slug),

    description text NOT NULL DEFAULT ''
);

CREATE TABLE odin.group_ledger (
    reference text NOT NULL,
    group_slug text NOT NULL,
    CONSTRAINT odin_group_ledger_group_fkey
        FOREIGN KEY (group_slug)
        REFERENCES odin.group (slug) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_group_ledger_pk PRIMARY KEY (reference, group_slug),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    description text NOT NULL DEFAULT '',

    annotation json NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.group_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.group (slug, description)
            VALUES (NEW.group_slug, NEW.description)
            ON CONFLICT (slug) DO UPDATE SET
                description = EXCLUDED.description;
        RETURN NEW;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_group_ledger_insert_trigger
    BEFORE INSERT ON odin.group_ledger
    FOR EACH ROW EXECUTE PROCEDURE odin.group_ledger_insert();


-- Users can be assigned to any number of groups
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
CREATE TABLE odin.group_membership_ledger (
    reference text NOT NULL,
    identity_id text NOT NULL,
    CONSTRAINT odin_group_membership_ledger_identity_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    group_slug text NOT NULL,
    CONSTRAINT odin_group_membership_ledger_group_fkey
        FOREIGN KEY (group_slug)
        REFERENCES odin.group (slug) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_group_membership_ledger_pk PRIMARY KEY
        (reference, identity_id, group_slug),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    -- Inserrt with true to add the membership, with false to remove it
    member boolean NOT NULL,

    annotation json NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.group_membership_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        IF NEW.member THEN
            INSERT INTO odin.group_membership (identity_id, group_slug)
                VALUES (NEW.identity_id, NEW.group_slug)
                ON CONFLICT DO NOTHING;
        ELSE
            DELETE FROM odin.group_membership
                WHERE identity_id=NEW.identity_id AND group_slug=NEW.group_slug;
        END if;
        RETURN NEW;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_group_membership_ledger_insert_trigger
    BEFORE INSERT ON odin.group_membership_ledger
    FOR EACH ROW EXECUTE PROCEDURE odin.group_membership_ledger_insert();


-- Permissions are application level names that control specific features
CREATE TABLE odin.permission (
    slug text NOT NULL CHECK (odin.url_safe(slug)),
    CONSTRAINT odin_permission_pk PRIMARY KEY (slug),

    description text NOT NULL DEFAULT ''
);

CREATE TABLE odin.permission_ledger (
    reference text NOT NULL,
    permission_slug text NOT NULL,
    CONSTRAINT odin_permission_ledger_permission_fkey
        FOREIGN KEY (permission_slug)
        REFERENCES odin.permission (slug) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    CONSTRAINT odin_permission_ledger_pk PRIMARY KEY (reference, permission_slug),

    changed timestamp with time zone NOT NULL DEFAULT now(),
    pg_user text NOT NULL DEFAULT current_user,

    description text NOT NULL DEFAULT '',

    annotation json NOT NULL DEFAULT '{}'
);
CREATE FUNCTION odin.permission_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.permission (slug, description)
            VALUES (NEW.permission_slug, NEW.description)
            ON CONFLICT (slug) DO UPDATE SET
                description = EXCLUDED.description;
        RETURN NEW;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
CREATE TRIGGER odin_permission_ledger_insert_trigger
    BEFORE INSERT ON odin.permission_ledger
    FOR EACH ROW EXECUTE PROCEDURE odin.permission_ledger_insert();


-- Groups are granted any number of permissions
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

