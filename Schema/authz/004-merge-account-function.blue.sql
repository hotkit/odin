INSERT INTO odin.migration VALUES('authz', '004-merge-account-function.blue.sql');

CREATE FUNCTION odin.merge_account_authz(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
DECLARE
  is_merge_from_superuser BOOLEAN;
  reference_val TEXT = 'merge:'||merge_from||':'||merge_to;
  annotation JSON = json_build_object('merge_from', merge_from,'merge_to', merge_to);
BEGIN
  SELECT is_superuser INTO is_merge_from_superuser FROM odin.identity WHERE id = merge_from;
  IF is_merge_from_superuser THEN
    INSERT INTO odin.identity_superuser_ledger (reference, identity_id, superuser)
    VALUES (reference_val, merge_to, is_merge_from_superuser);
  END IF;

  INSERT INTO odin.group_membership_ledger (reference, group_slug, identity_id, member, annotation)
  SELECT reference_val AS reference, group_slug, merge_to AS identity_id, True AS member, annotation AS annotation
  FROM odin.group_membership WHERE identity_id = merge_from;

  DELETE FROM odin.group_membership WHERE identity_id = merge_from;
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
