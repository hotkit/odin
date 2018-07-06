INSERT INTO odin.migration VALUES('authz', '002-view-user_permission.blue.sql') ON CONFLICT DO NOTHING;

DROP VIEW odin.user_permission;
DROP VIEW IF EXISTS odin.identity_permission;

CREATE MATERIALIZED VIEW odin.identity_permission AS
    SELECT DISTINCT
            odin.identity.id as identity_id,
            odin.permission.slug AS permission_slug,
            odin.permission.description
        FROM odin.identity
        JOIN odin.group_membership ON
            (odin.identity.id=odin.group_membership.identity_id
                 OR odin.identity.is_superuser)
        JOIN odin.group_grant ON
            (odin.group_grant.group_slug=odin.group_membership.group_slug
                 OR odin.identity.is_superuser)
        JOIN odin.permission ON
            (odin.permission.slug=odin.group_grant.permission_slug
                OR odin.identity.is_superuser);
CREATE INDEX identity_permission_identity_id_idx ON odin.identity_permission (identity_id);

CREATE VIEW odin.user_permission AS
    SELECT identity_id, permission_slug, description
    FROM odin.identity_permission identity_permission, odin.identity identity
    WHERE identity_permission.identity_id = identity.id
        AND (identity.expires IS NULL OR identity.expires > now());


CREATE OR REPLACE FUNCTION odin.identity_superuser_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.identity (id, is_superuser)
            VALUES (NEW.identity_id, NEW.superuser)
            ON CONFLICT (id) DO UPDATE SET
                is_superuser = EXCLUDED.is_superuser;
        REFRESH MATERIALIZED VIEW CONCURRENTLY odin.identity_permission;
        RETURN NULL;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE OR REPLACE FUNCTION odin.group_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.group (slug, description)
            VALUES (NEW.group_slug, NEW.description)
            ON CONFLICT (slug) DO UPDATE SET
                description = EXCLUDED.description;
        REFRESH MATERIALIZED VIEW CONCURRENTLY odin.identity_permission;
        RETURN NEW;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE OR REPLACE FUNCTION odin.group_membership_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        IF NEW.member THEN
            INSERT INTO odin.group_membership (identity_id, group_slug)
                VALUES (NEW.identity_id, NEW.group_slug)
                ON CONFLICT DO NOTHING;
        ELSE
            DELETE FROM odin.group_membership
                WHERE identity_id=NEW.identity_id AND group_slug=NEW.group_slug;
        END if;
        REFRESH MATERIALIZED VIEW CONCURRENTLY odin.identity_permission;
        RETURN NEW;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE OR REPLACE FUNCTION odin.permission_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        INSERT INTO odin.permission (slug, description)
            VALUES (NEW.permission_slug, NEW.description)
            ON CONFLICT (slug) DO UPDATE SET
                description = EXCLUDED.description;
        REFRESH MATERIALIZED VIEW CONCURRENTLY odin.identity_permission;
        RETURN NEW;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

CREATE OR REPLACE FUNCTION odin.group_grant_ledger_insert() RETURNS TRIGGER AS $body$
    BEGIN
        IF NEW.allows THEN
            INSERT INTO odin.group_grant (group_slug, permission_slug)
                VALUES (NEW.group_slug, NEW.permission_slug)
                ON CONFLICT DO NOTHING;
        ELSE
            DELETE FROM odin.group_grant
                WHERE group_slug = NEW.group_slug AND permission_slug = NEW.permission_slug;
        END if;
        REFRESH MATERIALIZED VIEW CONCURRENTLY odin.identity_permission;
        RETURN NEW;
    END
    $body$ LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
