#include "crewgle/Security.h"

#include <openssl/sha.h>

#include <algorithm>
#include <array>
#include <iomanip>
#include <random>
#include <sstream>

namespace crewgle {

std::string hashPassword(const std::string& password) {
    std::array<unsigned char, SHA256_DIGEST_LENGTH> hash{};
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash.data());

    std::ostringstream out;
    for (unsigned char byte : hash) {
        out << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return out.str();
}

std::string makeToken() {
    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<unsigned long long> dist;
    std::ostringstream out;
    for (int i = 0; i < 4; ++i) {
        out << std::hex << std::setw(16) << std::setfill('0') << dist(rng);
    }
    return out.str();
}

std::string bearerToken(const drogon::HttpRequestPtr& request) {
    const std::string prefix = "Bearer ";
    const std::string header = request->getHeader("Authorization");
    if (header.rfind(prefix, 0) != 0) {
        return "";
    }
    return header.substr(prefix.size());
}

bool roleIn(const Json::Value& user, const std::vector<std::string>& roles) {
    if (user.isNull() || !user.isMember("role")) {
        return false;
    }
    const std::string role = user["role"].asString();
    if (role == "Admin") {
        return true;
    }
    return std::find(roles.begin(), roles.end(), role) != roles.end();
}

}
