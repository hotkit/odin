if(TARGET stress OR TARGET pgtest)
    add_custom_command(OUTPUT test-authnz
            COMMAND fostgres-test odin-test-authnz -o test-authnz
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/authnz-test-view.json
                ${CMAKE_CURRENT_SOURCE_DIR}/authnz.fg
            MAIN_DEPENDENCY authnz.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/authz/001-initial.blue.sql
                ../Schema/authz/002-view-user_permission.blue.sql
                ../Schema/authz/003-alter-ledgers.blue.sql
                ../Schema/authz/004-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                authnz-test-view.json
                authnz.fg
        )

    add_custom_command(OUTPUT test-db-bypass
            COMMAND fostgres-test odin-test-db-bypass -o test-db-bypass
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/db-bypass.fg
            MAIN_DEPENDENCY db-bypass.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Configuration/odin-views.json
                db-bypass.fg
        )

    add_custom_command(OUTPUT test-login
            COMMAND fostgres-test odin-test-login -o test-login
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/login.fg
            MAIN_DEPENDENCY login.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/opts/installation-id/001-initial.blue.sql
                ../Schema/opts/installation-id/003-alter-ledgers.blue.sql
                ../Schema/opts/installation-id/004-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                login.fg
        )

    add_custom_command(OUTPUT test-logout
            COMMAND fostgres-test odin-test-logout -o test-logout
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/logout.fg
            MAIN_DEPENDENCY logout.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/opts/logout/002-initial.blue.sql
                ../Schema/opts/logout/003-fix-logout-count.blue.sql
                ../Schema/opts/logout/004-alter-ledgers.blue.sql
                ../Schema/opts/logout/005-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                logout.fg
        )

    add_custom_command(OUTPUT test-merge-account
            COMMAND fostgres-test odin-test-merge-account -o test-merge-account
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../views/merge.account.json
                ${CMAKE_CURRENT_SOURCE_DIR}/merge-account-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/merge-account.fg
            MAIN_DEPENDENCY merge-account.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/authz/001-initial.blue.sql
                ../Schema/authz/002-view-user_permission.blue.sql
                ../Schema/authz/003-alter-ledgers.blue.sql
                ../Schema/authz/004-merge-account-function.blue.sql
                ../Schema/app/002-initial.blue.sql
                ../Schema/app/003-app-role.blue.sql
                ../Schema/app/004-app-installation.blue.sql
                ../Schema/app/005-alter-ledgers.blue.sql
                ../Schema/app/006-merge-account-function.blue.sql
                ../Schema/app/007-app-user-data.blue.sql
                ../Schema/app/008-app-user-id.blue.sql
                ../Schema/app/009-app-user-id-index.blue.sql
                ../Schema/app/010-app-data-not-null.blue.sql
                ../Schema/opts/email/001-initial.blue.sql
                ../Schema/opts/email/003-alter-ledgers.blue.sql
                ../Schema/opts/email/004-merge-account-function.blue.sql
                ../Schema/opts/email/005-email-not-unique.blue.sql
                ../Schema/opts/facebook/001-initial.blue.sql
                ../Schema/opts/facebook/003-alter-ledgers.blue.sql
                ../Schema/opts/facebook/004-merge-account-function.blue.sql
                ../Schema/opts/facebook/005-facebook-credentials-insert.blue.sql
                ../Schema/opts/facebook/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/forgotten-password/001-initial.blue.sql
                ../Schema/opts/forgotten-password/002-merge-account-function.blue.sql
                ../Schema/opts/full-name/001-initial.blue.sql
                ../Schema/opts/full-name/003-alter-ledgers.blue.sql
                ../Schema/opts/full-name/004-merge-account-function.blue.sql
                ../Schema/opts/google/001-initial.blue.sql
                ../Schema/opts/google/003-alter-ledgers.blue.sql
                ../Schema/opts/google/004-merge-account-function.blue.sql
                ../Schema/opts/google/005-google-credentials-insert.blue.sql
                ../Schema/opts/google/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/installation-id/001-initial.blue.sql
                ../Schema/opts/installation-id/003-alter-ledgers.blue.sql
                ../Schema/opts/installation-id/004-merge-account-function.blue.sql
                ../Schema/opts/logout/002-initial.blue.sql
                ../Schema/opts/logout/003-fix-logout-count.blue.sql
                ../Schema/opts/logout/004-alter-ledgers.blue.sql
                ../Schema/opts/logout/005-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                ../views/merge.account.json
                merge-account.fg
        )

    add_custom_command(OUTPUT test-no-auth
            COMMAND fostgres-test odin-test-no-auth -o test-no-auth
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/no-auth.fg
            MAIN_DEPENDENCY no-auth.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Configuration/odin-views.json
                no-auth.fg
        )

    add_custom_command(OUTPUT test-password
            COMMAND fostgres-test odin-test-password -o test-password
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/password.fg
            MAIN_DEPENDENCY password.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/logout/002-initial.blue.sql
                ../Schema/opts/logout/003-fix-logout-count.blue.sql
                ../Schema/opts/logout/004-alter-ledgers.blue.sql
                ../Schema/opts/logout/005-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                password.fg
        )

    add_custom_command(OUTPUT test-permissions
            COMMAND fostgres-test odin-test-permissions -o test-permissions
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/permissions-test-view.json
                ${CMAKE_CURRENT_SOURCE_DIR}/permissions.fg
            MAIN_DEPENDENCY permissions.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/authz/001-initial.blue.sql
                ../Schema/authz/002-view-user_permission.blue.sql
                ../Schema/authz/003-alter-ledgers.blue.sql
                ../Schema/authz/004-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                permissions.fg
                permissions-test-view.json
        )

    add_custom_command(OUTPUT test-permissions-cookie
            COMMAND fostgres-test odin-test-permissions-cookie -o test-permissions-cookie
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/permissions-test-view.cookie.json
                ${CMAKE_CURRENT_SOURCE_DIR}/permissions.cookie.fg
            MAIN_DEPENDENCY permissions.cookie.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/authz/001-initial.blue.sql
                ../Schema/authz/002-view-user_permission.blue.sql
                ../Schema/authz/003-alter-ledgers.blue.sql
                ../Schema/authz/004-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                permissions.cookie.fg
                permissions-test-view.cookie.json
        )

    add_custom_command(OUTPUT test-registration-self
            COMMAND fostgres-test odin-test-registration-self -o test-registration-self
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/login-google-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/facebook-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/registration-self.fg
            MAIN_DEPENDENCY registration-self.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/full-name/001-initial.blue.sql
                ../Schema/opts/full-name/003-alter-ledgers.blue.sql
                ../Schema/opts/full-name/004-merge-account-function.blue.sql
                ../Schema/opts/email/001-initial.blue.sql
                ../Schema/opts/email/003-alter-ledgers.blue.sql
                ../Schema/opts/email/004-merge-account-function.blue.sql
                ../Schema/opts/email/005-email-not-unique.blue.sql
                ../Schema/opts/google/001-initial.blue.sql
                ../Schema/opts/google/003-alter-ledgers.blue.sql
                ../Schema/opts/google/004-merge-account-function.blue.sql
                ../Schema/opts/google/005-google-credentials-insert.blue.sql
                ../Schema/opts/google/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/facebook/001-initial.blue.sql
                ../Schema/opts/facebook/003-alter-ledgers.blue.sql
                ../Schema/opts/facebook/004-merge-account-function.blue.sql
                ../Schema/opts/facebook/005-facebook-credentials-insert.blue.sql
                ../Schema/opts/facebook/006-not-allow-merge-registered-to-guest.blue.sql
                ../Configuration/odin-views.json
                login-google-test.json
                facebook-test.json
                registration-self.fg
        )

    add_custom_command(OUTPUT test-session
            COMMAND fostgres-test odin-test-session -o test-session
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/session-test-view.json
                ${CMAKE_CURRENT_SOURCE_DIR}/session.fg
            MAIN_DEPENDENCY session.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Configuration/odin-views.json
                session.fg
                session-test-view.json
        )

    add_custom_command(OUTPUT test-forgotten-password
            COMMAND fostgres-test odin-test-forgotten-password -o test-forgotten-password
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/forgotten-password-test-view.json
                ${CMAKE_CURRENT_SOURCE_DIR}/forgotten-password.fg
            MAIN_DEPENDENCY forgotten-password.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/opts/forgotten-password/001-initial.blue.sql
                ../Schema/opts/forgotten-password/002-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                forgotten-password-test-view.json
                forgotten-password.fg
        )

    add_custom_command(OUTPUT test-link-account
            COMMAND fostgres-test odin-test-link-account -o test-link-account
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
                ${CMAKE_CURRENT_SOURCE_DIR}/link.account.fg
            MAIN_DEPENDENCY link.account.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/app/002-initial.blue.sql
                ../Schema/app/003-app-role.blue.sql
                ../Schema/app/004-app-installation.blue.sql
                ../Schema/app/005-alter-ledgers.blue.sql
                ../Schema/app/006-merge-account-function.blue.sql
                ../Schema/app/007-app-user-data.blue.sql
                ../Schema/app/008-app-user-id.blue.sql
                ../Schema/app/009-app-user-id-index.blue.sql
                ../Schema/app/010-app-data-not-null.blue.sql
                ../Schema/opts/facebook/001-initial.blue.sql
                ../Schema/opts/facebook/003-alter-ledgers.blue.sql
                ../Schema/opts/facebook/004-merge-account-function.blue.sql
                ../Schema/opts/facebook/005-facebook-credentials-insert.blue.sql
                ../Schema/opts/facebook/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/google/001-initial.blue.sql
                ../Schema/opts/google/003-alter-ledgers.blue.sql
                ../Schema/opts/google/004-merge-account-function.blue.sql
                ../Schema/opts/google/005-google-credentials-insert.blue.sql
                ../Schema/opts/google/006-not-allow-merge-registered-to-guest.blue.sql
                ../Configuration/odin-views.json
                link.account.fg
        )

    add_custom_command(OUTPUT test-login-facebook
            COMMAND fostgres-test odin-test-login-facebook -o test-login-facebook
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/facebook-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/login-google-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/login-facebook.fg
            MAIN_DEPENDENCY login-facebook.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/email/001-initial.blue.sql
                ../Schema/opts/email/003-alter-ledgers.blue.sql
                ../Schema/opts/email/004-merge-account-function.blue.sql
                ../Schema/opts/email/005-email-not-unique.blue.sql
                ../Schema/opts/facebook/001-initial.blue.sql
                ../Schema/opts/facebook/003-alter-ledgers.blue.sql
                ../Schema/opts/facebook/004-merge-account-function.blue.sql
                ../Schema/opts/facebook/005-facebook-credentials-insert.blue.sql
                ../Schema/opts/facebook/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/full-name/001-initial.blue.sql
                ../Schema/opts/full-name/003-alter-ledgers.blue.sql
                ../Schema/opts/full-name/004-merge-account-function.blue.sql
                ../Schema/opts/installation-id/001-initial.blue.sql
                ../Schema/opts/installation-id/003-alter-ledgers.blue.sql
                ../Schema/opts/google/001-initial.blue.sql
                ../Schema/opts/google/003-alter-ledgers.blue.sql
                ../Schema/opts/google/004-merge-account-function.blue.sql
                ../Schema/opts/google/005-google-credentials-insert.blue.sql
                ../Schema/opts/google/006-not-allow-merge-registered-to-guest.blue.sql
                ../Configuration/odin-views.json
                facebook-test.json
                login-google-test.json
                login-facebook.fg
        )

    add_custom_command(OUTPUT test-login-google
            COMMAND fostgres-test odin-test-login-google -o test-login-google
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
                ${CMAKE_CURRENT_SOURCE_DIR}/login-google-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/facebook-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/login-google.fg
            MAIN_DEPENDENCY login-google.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/opts/email/001-initial.blue.sql
                ../Schema/opts/email/003-alter-ledgers.blue.sql
                ../Schema/opts/email/004-merge-account-function.blue.sql
                ../Schema/opts/email/005-email-not-unique.blue.sql
                ../Schema/opts/google/001-initial.blue.sql
                ../Schema/opts/google/003-alter-ledgers.blue.sql
                ../Schema/opts/google/004-merge-account-function.blue.sql
                ../Schema/opts/google/005-google-credentials-insert.blue.sql
                ../Schema/opts/full-name/001-initial.blue.sql
                ../Schema/opts/full-name/003-alter-ledgers.blue.sql
                ../Schema/opts/full-name/004-merge-account-function.blue.sql
                ../Schema/opts/facebook/001-initial.blue.sql
                ../Schema/opts/facebook/003-alter-ledgers.blue.sql
                ../Schema/opts/facebook/004-merge-account-function.blue.sql
                ../Schema/opts/facebook/005-facebook-credentials-insert.blue.sql
                ../Schema/opts/facebook/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/installation-id/001-initial.blue.sql
                ../Schema/opts/installation-id/003-alter-ledgers.blue.sql
                ../Schema/opts/installation-id/004-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                login-google-test.json
                facebook-test.json
                login-google.fg
        )

    add_custom_command(OUTPUT test-expire-user
            COMMAND fostgres-test odin-test-expire-user -o test-expire-user
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
                ${CMAKE_CURRENT_SOURCE_DIR}/expire-user.fg
            MAIN_DEPENDENCY expire-user.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/logout/002-initial.blue.sql
                ../Schema/opts/logout/003-fix-logout-count.blue.sql
                ../Schema/opts/logout/004-alter-ledgers.blue.sql
                ../Schema/opts/logout/005-merge-account-function.blue.sql
                ../Configuration/odin-views.json
                expire-user.fg
        )

    add_custom_command(OUTPUT test-app-installation
            COMMAND fostgres-test odin-test-app-installation -o test-app-installation
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-federation-server.json
                ${CMAKE_CURRENT_SOURCE_DIR}/app-installation.fg
            MAIN_DEPENDENCY app-installation.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/app/002-initial.blue.sql
                ../Schema/app/003-app-role.blue.sql
                ../Schema/app/004-app-installation.blue.sql
                ../Schema/app/005-alter-ledgers.blue.sql
                ../Schema/app/006-merge-account-function.blue.sql
                ../Schema/app/007-app-user-data.blue.sql
                ../Schema/app/008-app-user-id.blue.sql
                ../Schema/app/009-app-user-id-index.blue.sql
                ../Schema/app/010-app-data-not-null.blue.sql
                ../Configuration/odin-views.json
                app-installation.fg
        )

    add_custom_command(OUTPUT test-app-mint
            COMMAND fostgres-test odin-test-app-mint -o test-app-mint
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-federation-server.json
                ${CMAKE_CURRENT_SOURCE_DIR}/app.mint-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/app.mint.fg
            MAIN_DEPENDENCY app.mint.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/app/002-initial.blue.sql
                ../Schema/app/003-app-role.blue.sql
                ../Schema/app/004-app-installation.blue.sql
                ../Schema/app/005-alter-ledgers.blue.sql
                ../Schema/app/006-merge-account-function.blue.sql
                ../Schema/app/007-app-user-data.blue.sql
                ../Schema/app/008-app-user-id.blue.sql
                ../Schema/app/009-app-user-id-index.blue.sql
                ../Schema/app/010-app-data-not-null.blue.sql
                ../Configuration/odin-views.json
                app.mint-test.json
                app.mint.fg
        )

    add_custom_command(OUTPUT test-app-login
        COMMAND fostgres-test odin-test-app-login -o test-app-login
            $<TARGET_SONAME_FILE:odin-fg>
            $<TARGET_SONAME_FILE:odin-views>
            ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
            ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
            ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
            ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-federation-server.json
            ${CMAKE_CURRENT_SOURCE_DIR}/app-login.fg
        MAIN_DEPENDENCY app-login.fg
        DEPENDS
            fostgres-test
            fostgres
            odin-fg
            odin-views
            ../Schema/bootstrap.sql
            ../Schema/core/000-initial.blue.sql
            ../Schema/core/002-add-merge-account.blue.sql
            ../Schema/authn/001-initial.blue.sql
            ../Schema/authn/002-fix-login.blue.sql
            ../Schema/authn/003-alter-ledgers.blue.sql
            ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
            ../Schema/app/002-initial.blue.sql
            ../Schema/app/003-app-role.blue.sql
            ../Schema/app/004-app-installation.blue.sql
            ../Schema/app/005-alter-ledgers.blue.sql
            ../Schema/app/006-merge-account-function.blue.sql
            ../Schema/app/007-app-user-data.blue.sql
            ../Schema/app/008-app-user-id.blue.sql
            ../Configuration/odin-views.json
            app-login.fg
    )

    add_custom_command(OUTPUT odin-test-app-login-facebook
            COMMAND fostgres-test odin-test-app-login-facebook -o odin-test-app-login-facebook
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/login-google-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/facebook-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/app-login-facebook.fg
            MAIN_DEPENDENCY app-login-facebook.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/opts/email/001-initial.blue.sql
                ../Schema/opts/email/003-alter-ledgers.blue.sql
                ../Schema/opts/email/004-merge-account-function.blue.sql
                ../Schema/opts/email/005-email-not-unique.blue.sql
                ../Schema/opts/facebook/001-initial.blue.sql
                ../Schema/opts/facebook/003-alter-ledgers.blue.sql
                ../Schema/opts/facebook/004-merge-account-function.blue.sql
                ../Schema/opts/facebook/005-facebook-credentials-insert.blue.sql
                ../Schema/opts/facebook/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/google/001-initial.blue.sql
                ../Schema/opts/google/003-alter-ledgers.blue.sql
                ../Schema/opts/google/004-merge-account-function.blue.sql
                ../Schema/opts/google/005-google-credentials-insert.blue.sql
                ../Schema/opts/google/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/full-name/001-initial.blue.sql
                ../Schema/opts/full-name/003-alter-ledgers.blue.sql
                ../Schema/opts/full-name/004-merge-account-function.blue.sql
                ../Schema/app/002-initial.blue.sql
                ../Schema/app/003-app-role.blue.sql
                ../Schema/app/004-app-installation.blue.sql
                ../Schema/app/005-alter-ledgers.blue.sql
                ../Schema/app/006-merge-account-function.blue.sql
                ../Schema/app/007-app-user-data.blue.sql
                ../Schema/app/008-app-user-id.blue.sql
                ../Schema/app/009-app-user-id-index.blue.sql
                ../Schema/app/010-app-data-not-null.blue.sql
                ../Configuration/odin-views.json
                facebook-test.json
                app-login-facebook.fg
        )

    add_custom_command(OUTPUT odin-test-app-login-google
            COMMAND fostgres-test odin-test-app-login-google -o odin-test-app-login-google
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
                ${CMAKE_CURRENT_SOURCE_DIR}/login-google-test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/app-login-google.fg
            MAIN_DEPENDENCY app-login-google.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/opts/email/001-initial.blue.sql
                ../Schema/opts/email/003-alter-ledgers.blue.sql
                ../Schema/opts/email/004-merge-account-function.blue.sql
                ../Schema/opts/email/005-email-not-unique.blue.sql
                ../Schema/opts/google/001-initial.blue.sql
                ../Schema/opts/google/003-alter-ledgers.blue.sql
                ../Schema/opts/google/004-merge-account-function.blue.sql
                ../Schema/opts/google/005-google-credentials-insert.blue.sql
                ../Schema/opts/google/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/facebook/001-initial.blue.sql
                ../Schema/opts/facebook/003-alter-ledgers.blue.sql
                ../Schema/opts/facebook/004-merge-account-function.blue.sql
                ../Schema/opts/facebook/005-facebook-credentials-insert.blue.sql
                ../Schema/opts/facebook/006-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/opts/full-name/001-initial.blue.sql
                ../Schema/opts/full-name/003-alter-ledgers.blue.sql
                ../Schema/opts/full-name/004-merge-account-function.blue.sql
                ../Schema/app/002-initial.blue.sql
                ../Schema/app/003-app-role.blue.sql
                ../Schema/app/004-app-installation.blue.sql
                ../Schema/app/005-alter-ledgers.blue.sql
                ../Schema/app/006-merge-account-function.blue.sql
                ../Schema/app/007-app-user-data.blue.sql
                ../Schema/app/008-app-user-id.blue.sql
                ../Schema/app/009-app-user-id-index.blue.sql
                ../Schema/app/010-app-data-not-null.blue.sql
                ../Configuration/odin-views.json
                login-google-test.json
                app-login-google.fg
        )

    add_custom_command(OUTPUT test-app-secure
        COMMAND fostgres-test odin-test-app-secure -o test-app-secure
            $<TARGET_SONAME_FILE:odin-fg>
            $<TARGET_SONAME_FILE:odin-views>
            ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
            ${CMAKE_CURRENT_SOURCE_DIR}/app-secure-test-view.json
            ${CMAKE_CURRENT_SOURCE_DIR}/app-secure.fg
        MAIN_DEPENDENCY app-secure.fg
        DEPENDS
            fostgres-test
            fostgres
            odin-fg
            odin-views
            ../Schema/bootstrap.sql
            ../Schema/core/000-initial.blue.sql
            ../Schema/core/002-add-merge-account.blue.sql
            ../Schema/authn/001-initial.blue.sql
            ../Schema/authn/002-fix-login.blue.sql
            ../Schema/authn/003-alter-ledgers.blue.sql
            ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
            ../Schema/app/002-initial.blue.sql
            ../Schema/app/003-app-role.blue.sql
            ../Schema/app/004-app-installation.blue.sql
            ../Schema/app/005-alter-ledgers.blue.sql
            ../Schema/app/006-merge-account-function.blue.sql
            ../Schema/app/007-app-user-data.blue.sql
            ../Schema/app/008-app-user-id.blue.sql
            ../Schema/opts/email/001-initial.blue.sql
            ../Schema/opts/email/003-alter-ledgers.blue.sql
            ../Schema/opts/email/004-merge-account-function.blue.sql
            ../Schema/opts/email/005-email-not-unique.blue.sql
            ../Schema/opts/facebook/001-initial.blue.sql
            ../Schema/opts/facebook/003-alter-ledgers.blue.sql
            ../Schema/opts/facebook/004-merge-account-function.blue.sql
            ../Schema/opts/facebook/005-facebook-credentials-insert.blue.sql
            ../Schema/opts/facebook/006-not-allow-merge-registered-to-guest.blue.sql
            ../Schema/opts/full-name/001-initial.blue.sql
            ../Schema/opts/full-name/003-alter-ledgers.blue.sql
            ../Schema/opts/full-name/004-merge-account-function.blue.sql
            ../Schema/opts/facebook/001-initial.blue.sql
            ../Schema/opts/facebook/003-alter-ledgers.blue.sql
            ../Schema/opts/facebook/004-merge-account-function.blue.sql
            ../Schema/opts/facebook/005-facebook-credentials-insert.blue.sql
            ../Schema/opts/facebook/006-not-allow-merge-registered-to-guest.blue.sql
            app-secure-test-view.json
            app-secure.fg
    )

    add_custom_command(OUTPUT test-app-verify
        COMMAND fostgres-test odin-test-app-verify -o test-app-verify
            $<TARGET_SONAME_FILE:odin-fg>
            $<TARGET_SONAME_FILE:odin-views>
            ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
            ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
            ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-federation-server.json
            ${CMAKE_CURRENT_SOURCE_DIR}/app-verify.fg
        MAIN_DEPENDENCY app-verify.fg
        DEPENDS
            fostgres-test
            fostgres
            odin-fg
            odin-views
            ../Schema/bootstrap.sql
            ../Schema/core/000-initial.blue.sql
            ../Schema/core/002-add-merge-account.blue.sql
            ../Schema/authn/001-initial.blue.sql
            ../Schema/authn/002-fix-login.blue.sql
            ../Schema/authn/003-alter-ledgers.blue.sql
            ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
            ../Schema/app/002-initial.blue.sql
            ../Schema/app/003-app-role.blue.sql
            ../Schema/app/004-app-installation.blue.sql
            ../Schema/app/005-alter-ledgers.blue.sql
            ../Schema/app/006-merge-account-function.blue.sql
            ../Schema/app/007-app-user-data.blue.sql
            ../Schema/app/008-app-user-id.blue.sql
            app-verify.fg
    )

    add_custom_command(OUTPUT test-app-handover
        COMMAND fostgres-test odin-test-app-handover -o test-app-handover
            $<TARGET_SONAME_FILE:odin-fg>
            $<TARGET_SONAME_FILE:odin-views>
            ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
            ${CMAKE_CURRENT_SOURCE_DIR}/app-data-test.json
            ${CMAKE_CURRENT_SOURCE_DIR}/app_user-app_data.fg
        MAIN_DEPENDENCY app-handover.fg
        DEPENDS
            fostgres-test
            fostgres
            odin-fg
            odin-views
            ../Schema/bootstrap.sql
            ../Schema/core/000-initial.blue.sql
            ../Schema/core/002-add-merge-account.blue.sql
            ../Schema/app/002-initial.blue.sql
            ../Schema/app/003-app-role.blue.sql
            ../Schema/app/004-app-installation.blue.sql
            ../Schema/app/005-alter-ledgers.blue.sql
            ../Schema/app/006-merge-account-function.blue.sql
            ../Schema/app/007-app-user-data.blue.sql
            ../Schema/app/008-app-user-id.blue.sql
            app-data-test.json
            app_user-app_data.fg
    )

    add_custom_command(OUTPUT test-app-handover
        COMMAND fostgres-test odin-test-app-handover -o test-app-handover
            $<TARGET_SONAME_FILE:odin-fg>
            $<TARGET_SONAME_FILE:odin-views>
            ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
            ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
            ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
            ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-federation-server.json
            ${CMAKE_CURRENT_SOURCE_DIR}/app-federation-test-client-view.json
            ${CMAKE_CURRENT_SOURCE_DIR}/app-handover.fg
        MAIN_DEPENDENCY app-handover.fg
        DEPENDS
            fostgres-test
            fostgres
            odin-fg
            odin-views
            ../Schema/bootstrap.sql
            ../Schema/core/000-initial.blue.sql
            ../Schema/core/002-add-merge-account.blue.sql
            ../Schema/authn/001-initial.blue.sql
            ../Schema/authn/002-fix-login.blue.sql
            ../Schema/authn/003-alter-ledgers.blue.sql
            ../Schema/authz/001-initial.blue.sql
            ../Schema/authz/002-view-user_permission.blue.sql
            ../Schema/authz/003-alter-ledgers.blue.sql
            ../Schema/authz/004-merge-account-function.blue.sql
            ../Schema/app/002-initial.blue.sql
            ../Schema/app/003-app-role.blue.sql
            ../Schema/app/004-app-installation.blue.sql
            ../Schema/app/005-alter-ledgers.blue.sql
            ../Schema/app/006-merge-account-function.blue.sql
            ../Schema/app/007-app-user-data.blue.sql
            ../Schema/app/008-app-user-id.blue.sql
            ../Schema/opts/full-name/001-initial.blue.sql
            ../Schema/opts/full-name/003-alter-ledgers.blue.sql
            ../Schema/opts/full-name/004-merge-account-function.blue.sql
            ../Schema/opts/email/001-initial.blue.sql
            ../Schema/opts/email/003-alter-ledgers.blue.sql
            ../Schema/opts/email/004-merge-account-function.blue.sql
            ../Schema/opts/email/005-email-not-unique.blue.sql
            app-federation-test-client-view.json
            app-handover.fg
    )

    add_custom_command(OUTPUT test-jwt-renewal
            COMMAND fostgres-test odin-test-jwt-renewal -o test-jwt-renewal
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-federation-server.json
                ${CMAKE_CURRENT_SOURCE_DIR}/jwt-renewal.fg
            MAIN_DEPENDENCY jwt-renewal.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/app/002-initial.blue.sql
                ../Schema/app/003-app-role.blue.sql
                ../Schema/app/004-app-installation.blue.sql
                ../Schema/app/005-alter-ledgers.blue.sql
                ../Schema/app/006-merge-account-function.blue.sql
                ../Schema/app/007-app-user-data.blue.sql
                ../Schema/app/008-app-user-id.blue.sql
                ../Schema/app/009-app-user-id-index.blue.sql
                ../Schema/app/010-app-data-not-null.blue.sql
                ../Configuration/odin-views.json
                jwt-renewal.fg
        )

    add_custom_command(OUTPUT test-registration-app-user
            COMMAND fostgres-test odin-test-registration-app-user -o test-registration-app-user
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-webserver.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-federation-server.json
                ${CMAKE_CURRENT_SOURCE_DIR}/registration-app-user.fg
            MAIN_DEPENDENCY registration-app-user.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/core/002-add-merge-account.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authn/003-alter-ledgers.blue.sql
                ../Schema/authn/004-not-allow-merge-registered-to-guest.blue.sql
                ../Schema/authz/001-initial.blue.sql
                ../Schema/authz/002-view-user_permission.blue.sql
                ../Schema/authz/003-alter-ledgers.blue.sql
                ../Schema/authz/004-merge-account-function.blue.sql
                ../Schema/app/002-initial.blue.sql
                ../Schema/app/003-app-role.blue.sql
                ../Schema/app/004-app-installation.blue.sql
                ../Schema/app/005-alter-ledgers.blue.sql
                ../Schema/app/006-merge-account-function.blue.sql
                ../Schema/app/007-app-user-data.blue.sql
                ../Schema/app/008-app-user-id.blue.sql
                ../Schema/app/009-app-user-id-index.blue.sql
                ../Schema/app/010-app-data-not-null.blue.sql
                ../Configuration/odin-views.json
                registration-app-user.fg
        )

    ## Because of the way cmake works we need this stuff at the end to
    ## actually make the above commands run when things change.
    add_custom_target(odin-tests DEPENDS
            test-app-handover
            test-app-installation
            test-app-mint
            test-app-login
            odin-test-app-login-facebook
            odin-test-app-login-google
            test-app-secure
            test-app-verify
            test-authnz
            test-db-bypass
            test-login
            test-logout
            test-merge-account
            test-no-auth
            test-password
            test-permissions
            test-permissions-cookie
            test-registration-self
            test-session
            test-forgotten-password
            test-link-account
            test-login-facebook
            test-login-google
            test-expire-user
            test-jwt-renewal
            test-registration-app-user
        )
    if(TARGET stress)
        add_dependencies(stress odin-tests)
    endif()
    if(TARGET pgtest)
        add_dependencies(pgtest odin-tests)
    endif()
endif()
