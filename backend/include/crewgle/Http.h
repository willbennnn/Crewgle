#pragma once

#include <drogon/drogon.h>

#include <functional>
#include <string>
#include <vector>

namespace crewgle {

void addCors(const drogon::HttpResponsePtr& response);
drogon::HttpResponsePtr jsonResponse(const Json::Value& body, drogon::HttpStatusCode status = drogon::k200OK);
drogon::HttpResponsePtr errorResponse(const std::string& message, drogon::HttpStatusCode status);
Json::Value requireJson(const drogon::HttpRequestPtr& request);
void requireFields(const Json::Value& json, const std::vector<std::string>& fields);
std::string optionalText(const Json::Value& json, const std::string& key, const std::string& fallback = "");
std::string queryParam(const drogon::HttpRequestPtr& request, const std::string& key, const std::string& fallback = "");

template <typename Fn>
void respond(Fn&& fn, std::function<void(const drogon::HttpResponsePtr&)>& callback) {
    try {
        callback(fn());
    } catch (const std::invalid_argument& e) {
        callback(errorResponse(e.what(), drogon::k400BadRequest));
    } catch (const std::out_of_range& e) {
        callback(errorResponse(e.what(), drogon::k404NotFound));
    } catch (const std::exception& e) {
        callback(errorResponse(e.what(), drogon::k500InternalServerError));
    }
}

}
