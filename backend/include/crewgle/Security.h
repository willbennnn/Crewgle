#pragma once

#include <drogon/drogon.h>

#include <string>
#include <vector>

namespace crewgle {

std::string hashPassword(const std::string& password);
std::string makeToken();
std::string bearerToken(const drogon::HttpRequestPtr& request);
bool roleIn(const Json::Value& user, const std::vector<std::string>& roles);

}
