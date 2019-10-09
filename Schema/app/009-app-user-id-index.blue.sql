
INSERT INTO odin.migration VALUES('app', '009-app-user-id-index.blue.sql');

CREATE INDEX app_user_app_id_app_user_id_idx ON odin.app_user (app_id, app_user_id);
