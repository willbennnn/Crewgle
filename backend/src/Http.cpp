#include "crewgle/Http.h"

#include <stdexcept>

namespace crewgle {

void addCors(const drogon::HttpResponsePtr& response) {
    response->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PATCH, DELETE, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

drogon::HttpResponsePtr jsonResponse(const Json::Value& body, drogon::HttpStatusCode status) {
    auto response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(status);
    addCors(response);
    return response;
}

drogon::HttpResponsePtr errorResponse(const std::string& message, drogon::HttpStatusCode status) {
    Json::Value body;
    body["error"] = message;
    return jsonResponse(body, status);
}

Json::Value requireJson(const drogon::HttpRequestPtr& request) {
    auto json = request->getJsonObject();
    if (!json) {
        throw std::invalid_argument("Expected a JSON body.");
    }
    return *json;
}

void requireFields(const Json::Value& json, const std::vector<std::string>& fields) {
    for (const auto& field : fields) {
        if (!json.isMember(field) || json[field].isNull()) {
            throw std::invalid_argument("Missing required field: " + field);
        }
    }
}

std::string optionalText(const Json::Value& json, const std::string& key, const std::string& fallback) {
    return json.isMember(key) && !json[key].isNull() ? json[key].asString() : fallback;
}

std::string queryParam(const drogon::HttpRequestPtr& request, const std::string& key, const std::string& fallback) {
    if (!request) {
        return fallback;
    }
    const std::string value = request->getParameter(key);
    return value.empty() ? fallback : value;
}

}
