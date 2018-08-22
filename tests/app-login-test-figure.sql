-- SETUP Mock Application

INSERT INTO odin.app_ledger (reference, app_id, app_name, redirect_url)
    VALUES ('ref1', 'app01', 'MyApp', 'http://example.com');

INSERT INTO odin.app_owner_ledger (reference, identity_id, app_id)
    VALUES ('ref1', 'owner', 'app01');

INSERT INTO odin.app_user_ledger (reference, identity_id, app_id)
    VALUES ('ref2', 'player1', 'app01');