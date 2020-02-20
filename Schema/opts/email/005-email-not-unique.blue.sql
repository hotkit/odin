INSERT INTO odin.migration VALUES('opts/email', '005-email-not-unique.blue.sql');

ALTER TABLE odin.identity DROP CONSTRAINT identity_email_key;
CREATE INDEX ON odin.identity (email);