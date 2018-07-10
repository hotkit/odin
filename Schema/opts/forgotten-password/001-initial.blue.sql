INSERT INTO odin.module VALUES('opts/forgotten-password') ON CONFLICT (name) DO NOTHING;
INSERT INTO odin.migration VALUES('opts/forgotten-password', '001-initial.blue.sql');


CREATE TABLE odin.request_reset_password_ledger (
    reference text NOT NULL,
    email text NOT NULL,
    request_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    sent_at TIMESTAMP WITH TIME ZONE,
    remark TEXT,
    PRIMARY KEY (reference, email)
);

