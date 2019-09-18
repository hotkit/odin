INSERT INTO odin.migration
VALUES('app', '008-app-user-appuserid.blue.sql');

ALTER TABLE odin.app_user ADD COLUMN app_user_id TEXT;