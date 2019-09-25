INSERT INTO odin.migration VALUES('opts/facebook', '006-not-allow-merge-registerd-to-guest.blue.sql');

CREATE OR REPLACE FUNCTION odin."merge_account_opts/facebook"(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;
