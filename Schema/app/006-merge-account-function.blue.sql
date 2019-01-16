INSERT INTO odin.migration VALUES('app', '006-merge-account-function.blue.sql');

ALTER TABLE odin.app_user_role_ledger
    DROP CONSTRAINT app_user_role_ledger_app_id_fkey,
    ADD CONSTRAINT app_user_role_ledger_app_id_fkey
        FOREIGN KEY (app_id)
        REFERENCES odin.app (app_id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    ADD CONSTRAINT app_user_role_ledger_identity_id_fkey
        FOREIGN KEY (identity_id)
        REFERENCES odin.identity_record (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;

CREATE FUNCTION odin.merge_account_app(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
DECLARE
  merge_to_installation_id TEXT;
  reference_val TEXT = 'merge:'||merge_from||':'||merge_to;
  annotation JSON = json_build_object('merge_from', merge_from,'merge_to', merge_to);
BEGIN
  SELECT installation_id INTO merge_to_installation_id FROM odin.app_user WHERE identity_id = merge_to LIMIT 1;

  INSERT INTO odin.app_user_ledger (reference, app_id, identity_id, annotation)
  SELECT reference_val AS reference, app_id, merge_to AS identity_id, annotation AS annotation
  FROM odin.app_user WHERE identity_id = merge_from;

  INSERT INTO odin.app_user_installation_id_ledger (reference, app_id, identity_id, installation_id, annotation)
  SELECT reference_val AS reference, app_id, merge_to AS identity_id, merge_to_installation_id AS installation_id, annotation AS annotation
  FROM odin.app_user WHERE identity_id = merge_from;

  INSERT INTO odin.app_user_role_ledger (reference, app_id, identity_id, role, annotation)
  SELECT reference_val AS reference, app_id, merge_to AS identity_id, role, annotation AS annotation
  FROM odin.app_user_role WHERE identity_id = merge_from;

  DELETE FROM odin.app_user_installation_id_ledger WHERE identity_id = merge_from;
  DELETE FROM odin.app_user_role WHERE identity_id = merge_from;
  DELETE FROM odin.app_user WHERE identity_id = merge_from;

END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
