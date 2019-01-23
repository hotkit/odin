INSERT INTO odin.migration VALUES('opts/installation-id', '004-merge-account-function.blue.sql');

CREATE FUNCTION odin."merge_account_opts/installation-id"(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
