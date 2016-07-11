

CREATE TABLE odin_password_ledger
(
    user_id text NOT NULL,
    CONSTRAINT password_user_fkey
        FOREIGN KEY (user_id)
        REFERENCES odin_users (id) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    set timestamp with time zone NOT NULL DEFAULT now(),
    CONSTRAINT password_pk PRIMARY KEY (user_id, set),

    hash text NOT NULL
);
