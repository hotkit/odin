/**
    Copyright 2018-2019, Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <odin/odin.hpp>
#include <odin/pwhashproc.hpp>

#include <fost/crypto>
#include <fost/insert>


namespace {


    std::vector<unsigned char>
            pbkdf2(f5::u8view password, fostlib::json procedure) {
        const auto salt = fostlib::coerce<std::vector<unsigned char>>(
                fostlib::base64_string(
                        fostlib::coerce<fostlib::string>(procedure["salt"])));
        const auto rounds = fostlib::coerce<int64_t>(procedure["rounds"]);
        const auto length = fostlib::coerce<int64_t>(procedure["length"]);
        const auto hashed =
                fostlib::pbkdf2_hmac_sha256(password, salt, rounds, length);
        return hashed;
    }


    std::vector<unsigned char>
            ripemd(fostlib::string password, fostlib::json procedure) {
        const auto salt = fostlib::coerce<f5::u8view>(procedure["salt"]);
        const auto hashed = fostlib::ripemd256(password + salt);
        std::vector<unsigned char> ret;
        for (auto c : hashed) ret.push_back(c);
        return ret;
    }


    std::vector<unsigned char>
            hash(f5::u8view password, fostlib::json procedure) {
        const auto object = [](f5::u8view pw, fostlib::json process) {
            const auto name = fostlib::coerce<f5::u8view>(process["name"]);
            if (name == "pbkdf2-sha256") {
                return pbkdf2(pw, process);
            } else if (name == "ripemd256") {
                return ripemd(pw, process);
            } else {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "Unknown hash function name",
                        name);
            }
        };
        if (procedure.isobject()) {
            return object(password, procedure);
        } else if (procedure.isarray()) {
            if (procedure.size() == 0) {
                throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "Empty array", procedure);
            }
            auto result = object(password, procedure[0]);
            for (size_t i = 1; i < procedure.size(); ++i) {
                result = object(f5::u8view(fostlib::coerce<fostlib::base64_string>(result)), procedure[i]);
            }
            return result;
        } else {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "Unknown procedure type", procedure);
        }
    }

}


bool odin::check_password(
        const fostlib::string &password,
        const fostlib::string &hash,
        const fostlib::json &procedure) {
    const auto hashb = fostlib::coerce<std::vector<unsigned char>>(
            fostlib::base64_string(hash));
    const auto hashed = ::hash(password, procedure);
    return fostlib::crypto_compare(hashed, hashb);
}


std::pair<fostlib::string, fostlib::json>
        odin::hash_password(f5::u8view password) {
    auto salt = fostlib::crypto_bytes<24>();
    fostlib::json process;
    fostlib::insert(process, "name", "pbkdf2-sha256");
    fostlib::insert(process, "rounds", c_hash_rounds.value());
    fostlib::insert(process, "length", 32);
    fostlib::insert(
            process, "salt",
            fostlib::coerce<fostlib::base64_string>(
                    std::vector<unsigned char>(salt.begin(), salt.end())));
    auto hashed = fostlib::string(
            fostlib::coerce<fostlib::base64_string>(hash(password, process)));
    return std::make_pair(hashed, process);
}
