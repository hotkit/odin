INSERT INTO odin.migration VALUES('authz', '004-merge-account-function.blue.sql');

CREATE FUNCTION odin.merge_account_authz(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
DECLARE
  is_merge_from_superuser BOOLEAN;
  is_merge_to_superuser BOOLEAN;
  reference_val TEXT = 'merge:'||merge_from||':'||merge_to;
  annotation JSON = json_build_object('merge_from', merge_from,'merge_to', merge_to);
BEGIN
  SELECT is_superuser INTO is_merge_from_superuser FROM odin.identity WHERE id = merge_from;
  SELECT is_superuser INTO is_merge_to_superuser FROM odin.identity WHERE id = merge_to;
  IF is_merge_from_superuser OR is_merge_to_superuser THEN
    RAISE 'Merge superuser not allow';
  ELSE
    UPDATE odin.group_membership
    SET identity_id=merge_to
    WHERE identity_id=merge_from;
  END IF;
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
