-- SETUP Mock Application
INSERT INTO odin.identity (id) VALUES ('app01'), ('app02');

INSERT INTO odin.app_access_policy VALUES ('OPEN');
INSERT INTO odin.app_ledger (reference, app_id, app_name, token, redirect_url, access_policy, data_sharing_policy) VALUES
    ('ref1', 'app01', 'MyApp', 'APP_TOKEN', 'http://example.com', 'INVITE_ONLY', 'ALL'),
    ('ref2', 'app02', 'MyApp2', 'APP2_TOKEN', 'http://example2.com', 'OPEN', 'ALL');

INSERT INTO odin.app_user_ledger (reference, app_id, identity_id, state)
    VALUES ('ref2', 'app01', 'player1', 'ACTIVE'), ('ref3', 'app01', 'player3', 'ACTIVE');

INSERT INTO odin.app_role_ledger (reference, app_id, role) VALUES ('test-app', 'app01', 'pro-player'), ('test-app', 'app01', 'noob');
INSERT INTO odin.app_user_role_ledger (reference, identity_id, app_id, role) VALUES ('test-app', 'player1', 'app01', 'pro-player');
