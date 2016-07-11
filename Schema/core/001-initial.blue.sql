

CREATE TABLE odin_users
(
    id text NOT NULL,
    CONSTRAINT users_pk PRIMARY KEY (id)
);


CREATE TABLE odin_groups
(
    id text NOT NULL,
    CONSTRAINT group_pk PRIMARY KEY (id)
);
