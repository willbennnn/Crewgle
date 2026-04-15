#include <drogon/drogon.h>

int main() {
    // /health route
    drogon::app().registerHandler(
        "/health",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value json;
            json["status"] = "ok";

            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);

            // CORS header
            resp->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");

            callback(resp);
        },
        {drogon::Get}
    );

    // /api/test route
    drogon::app().registerHandler(
        "/api/test",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value json;
            json["message"] = "backend is working";
            json["project"] = "crewgle";
            json["version"] = 1;

            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);

            // CORS header
            resp->addHeader("Access-Control-Allow-Origin", "http://localhost:5173");

            callback(resp);
        },
        {drogon::Get}
    );

    // Load config (keep your full path for now)
    drogon::app().loadConfigFile("/Users/willgallegos/Crewgle/backend/config.json");

    drogon::app().run();
}