#include "crewgle/Routes.h"
#include "crewgle/Schema.h"

#include <drogon/drogon.h>

#include <cstdlib>
#include <iostream>

int main() {
    const char* envPath = std::getenv("CREWGLE_DB_PATH");
    const std::string dbPath = envPath == nullptr ? "crewgle.sqlite" : envPath;

    auto db = std::make_shared<crewgle::Database>(dbPath);
    crewgle::migrate(*db);
    crewgle::seed(*db);

    auto services = std::make_shared<crewgle::Services>(db);
    crewgle::registerRoutes(services);

    drogon::app().addListener("0.0.0.0", 8080);
    std::cout << "Crewgle API running on http://localhost:8080 using " << dbPath << "\n";
    drogon::app().run();
}
