#include "crewgle/Routes.h"

#include "crewgle/Http.h"

namespace crewgle {

namespace {
Json::Value requireUser(Services& services, const drogon::HttpRequestPtr& request) {
    Json::Value user = services.currentUser(request);
    if (user.isNull()) {
        throw std::invalid_argument("Unauthorized.");
    }
    return user;
}
}

void registerRoutes(const std::shared_ptr<Services>& services) {
    using namespace drogon;

    app().registerPreRoutingAdvice([](const HttpRequestPtr& request, AdviceCallback&& callback, AdviceChainCallback&& chain) {
        if (request->method() == Options) {
            auto response = HttpResponse::newHttpResponse();
            addCors(response);
            callback(response);
            return;
        }
        chain();
    });

    app().registerHandler("/api/health", [](const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&& callback) {
        Json::Value body;
        body["service"] = "crewgle";
        body["status"] = "ok";
        callback(jsonResponse(body));
    }, {Get});

    app().registerHandler("/api/auth/register", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->registerUser(requireJson(request)), k201Created); }, callback);
    }, {Post});

    app().registerHandler("/api/auth/login", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->login(requireJson(request))); }, callback);
    }, {Post});

    app().registerHandler("/api/auth/me", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        Json::Value user = services->currentUser(request);
        if (user.isNull()) {
            callback(errorResponse("Unauthorized.", k401Unauthorized));
            return;
        }
        Json::Value body;
        body["user"] = user;
        callback(jsonResponse(body));
    }, {Get});

    app().registerHandler("/api/auth/logout", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        services->logout(request);
        Json::Value body;
        body["status"] = "logged out";
        callback(jsonResponse(body));
    }, {Post});

    app().registerHandler("/api/auth/reset-password", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] {
            return jsonResponse(services->forgotPassword(requireJson(request)));
        }, callback);
    }, {Post});

    app().registerHandler("/api/auth/reset-password/confirm", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] {
            return jsonResponse(services->resetPassword(requireJson(request)));
        }, callback);
    },
    {Post});

    app().registerHandler("/api/profile", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->updateProfile(requireUser(*services, request), requireJson(request))); }, callback);
    }, {Patch});

    app().registerHandler("/api/invitations", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->inviteUser(requireUser(*services, request), requireJson(request)), k201Created); }, callback);
    }, {Post});

    app().registerHandler("/api/bootstrap", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->appBootstrap(requireUser(*services, request))); }, callback);
    }, {Get});

    app().registerHandler("/api/users", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] {
            requireUser(*services, request);
            return jsonResponse(services->listUsers());
        }, callback);
    }, {Get});

    app().registerHandler("/api/squads", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] {
            requireUser(*services, request);
            return jsonResponse(services->listSquads());
        }, callback);
    }, {Get});

    app().registerHandler("/api/practice-templates", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] {
            requireUser(*services, request);
            return jsonResponse(services->listPracticeTemplates());
        }, callback);
    }, {Get});

    app().registerHandler("/api/practice-templates", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->createPracticeTemplate(requireUser(*services, request), requireJson(request)), k201Created); }, callback);
    }, {Post});

    app().registerHandler("/api/practice-templates/{1}/generate-week", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback, int templateId) {
        respond([&] { return jsonResponse(services->generateWeek(requireUser(*services, request), templateId, requireJson(request)), k201Created); }, callback);
    }, {Post});

    app().registerHandler("/api/weeks/{1}", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& weekStart) {
        respond([&] {
            requireUser(*services, request);
            return jsonResponse(services->weekOverview(weekStart));
        }, callback);
    }, {Get});

    app().registerHandler("/api/practices/{1}", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback, int practiceId) {
        respond([&] {
            return jsonResponse(services->practiceDetail(requireUser(*services, request), practiceId));
        }, callback);
    }, {Get});

    app().registerHandler("/api/practices", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->createPractice(requireUser(*services, request), requireJson(request)), k201Created); }, callback);
    }, {Post});

    app().registerHandler("/api/practices/{1}", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback, int practiceId) {
        respond([&] { return jsonResponse(services->deletePractice(requireUser(*services, request), practiceId)); }, callback);
    }, {Delete});

    app().registerHandler("/api/availability", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->setAvailability(requireUser(*services, request), requireJson(request))); }, callback);
    }, {Post});

    app().registerHandler("/api/lineups", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->createLineup(requireUser(*services, request), requireJson(request)), k201Created); }, callback);
    }, {Post});

    app().registerHandler("/api/lineups/{1}/substitutions", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback, int lineupId) {
        respond([&] { return jsonResponse(services->substituteLineupSeat(requireUser(*services, request), lineupId, requireJson(request))); }, callback);
    }, {Post});

    app().registerHandler("/api/substitution-requests", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->requestSubstitution(requireUser(*services, request), requireJson(request)), k201Created); }, callback);
    }, {Post});

    app().registerHandler("/api/substitution-requests/{1}", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback, int requestId) {
        respond([&] { return jsonResponse(services->respondToSubstitution(requireUser(*services, request), requestId, requireJson(request))); }, callback);
    }, {Patch});

    app().registerHandler("/api/lineups/week/{1}", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& weekStart) {
        respond([&] {
            requireUser(*services, request);
            return jsonResponse(services->weeklyLineupBoard(weekStart));
        }, callback);
    }, {Get});

    app().registerHandler("/api/attendance", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->recordAttendance(requireUser(*services, request), requireJson(request))); }, callback);
    }, {Post});

    app().registerHandler("/api/attendance/report", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->attendanceReport(requireUser(*services, request), request)); }, callback);
    }, {Get});

    app().registerHandler("/api/makeups", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->assignMakeup(requireUser(*services, request), requireJson(request)), k201Created); }, callback);
    }, {Post});

    app().registerHandler("/api/makeups/{1}", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback, int makeupId) {
        respond([&] { return jsonResponse(services->updateMakeup(requireUser(*services, request), makeupId, requireJson(request))); }, callback);
    }, {Patch});

    app().registerHandler("/api/workouts", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->createWorkout(requireUser(*services, request), requireJson(request)), k201Created); }, callback);
    }, {Post});

    app().registerHandler("/api/workouts/summary", [services](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        respond([&] { return jsonResponse(services->workoutSummary(requireUser(*services, request), request)); }, callback);
    }, {Get});
}

}
