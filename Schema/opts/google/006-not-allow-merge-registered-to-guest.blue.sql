/* Guest user is non credential user */
INSERT INTO odin.migration VALUES('opts/google', '006-not-allow-merge-registered-to-guest.blue.sql');

CREATE OR REPLACE FUNCTION odin."merge_account_opts/google"(merge_from TEXT, merge_to TEXT)
RETURNS VOID AS
$body$
BEGIN
END;
$body$
LANGUAGE plpgsql SECURITY DEFINER SET search_path = odin;

/* Old version is reference to identity record it incorrect */
ALTER TABLE odin.google_credentials ADD CONSTRAINT google_credentials_identity_fkey
    FOREIGN KEY (identity_id)
    REFERENCES odin.identity (id) MATCH SIMPLE
    ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE;
