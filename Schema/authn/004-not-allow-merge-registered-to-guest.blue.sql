INSERT INTO odin.migration VALUES('authn', '004-not-allow-merge-registered-to-guest.blue.sql');

CREATE OR REPLACE FUNCTION odin.merge_account_authn(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
