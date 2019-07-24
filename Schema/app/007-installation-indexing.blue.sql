INSERT INTO odin.migration VALUES('app', '007-installation-indexing.blue.sql');

CREATE INDEX IF NOT EXISTS idx_app_installation_id ON odin.app_user_installation_id_ledger (app_id, installation_id);
