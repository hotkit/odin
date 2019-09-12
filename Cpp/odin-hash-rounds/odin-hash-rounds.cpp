/**
    Copyright 2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fost/main>
#include <fost/timer>

#include <odin/pwhashproc.hpp>


FSL_MAIN("odin-hash-rounds", "Password hashing timer")
(fostlib::ostream &out, fostlib::arguments &args) {
    double const target_time = 0.3;
    int64_t const rounds = odin::c_hash_rounds.value();
    int64_t new_rounds = 0u, final_rounds = 0u;
    {
        fostlib::timer time;
        auto const hashed = odin::hash_password("password");
        auto const seconds = time.seconds();
        out << "Base hash time was " << seconds << "s for " << rounds
            << " rounds of "
            << fostlib::coerce<fostlib::string>(hashed.second["name"]) << '\n';
        new_rounds = rounds / seconds * target_time;
    }
    {
        out << "Timing " << new_rounds << " rounds....\n";
        fostlib::setting<int64_t> nr = {"odin-hash-rounds.cpp",
                                        odin::c_hash_rounds, new_rounds};
        fostlib::timer time;
        odin::hash_password("password");
        auto const seconds = time.seconds();
        out << "New time is " << seconds << "\n";
        final_rounds = new_rounds / seconds * target_time;
    }
    fostlib::json rec;
    fostlib::insert(
            rec, odin::c_hash_rounds.section(), odin::c_hash_rounds.name(),
            final_rounds);
    out << "Recommend:\n\n[" << odin::c_hash_rounds.section() << "]\n"
        << odin::c_hash_rounds.name() << "=" << final_rounds << "\n\n"
        << rec;
    return 0;
}
