
INSERT INTO odin.migration VALUES('app', '010-app-data-not-null.blue.sql');

ALTER TABLE odin.app_user ALTER COLUMN app_data DROP NOT NULL;
