CREATE SCHEMA odin;


CREATE TABLE odin.module (
    name text NOT NULL,
    CONSTRAINT module_pk PRIMARY KEY(name)
);

CREATE TABLE odin.migration (
    module_name text NOT NULL,
    CONSTRAINT migration_module_fkey
        FOREIGN KEY (module_name)
        REFERENCES odin.module (name) MATCH SIMPLE
        ON UPDATE NO ACTION ON DELETE NO ACTION DEFERRABLE,
    migration text NOT NULL,
    CONSTRAINT migration_pk PRIMARY KEY (module_name, migration)
);


-- Always enable this module
INSERT INTO odin.module VALUES ('core');
