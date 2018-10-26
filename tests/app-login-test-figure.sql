-- SETUP Mock Application
INSERT INTO odin.identity (id) VALUES ('app01'), ('app02');

INSERT INTO odin.app_ledger (reference, app_id, app_name, token, redirect_url, access_policy) VALUES
    ('ref1', 'app01', 'MyApp', 'APP_TOKEN', 'http://example.com', 'INVITE_ONLY'),
    ('ref2', 'app02', 'MyApp2', 'APP2_TOKEN', 'http://example2.com', 'OPEN');

INSERT INTO odin.app_owner_ledger (reference, identity_id, app_id)
    VALUES ('ref1', 'owner', 'app01');

INSERT INTO odin.app_user_ledger (reference, identity_id, app_id)
    VALUES ('ref2', 'player1', 'app01');
