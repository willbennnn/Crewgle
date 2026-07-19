#pragma once

#include "crewgle/Database.h"

#include <drogon/drogon.h>

#include <string>

namespace crewgle {

class Services {
public:
    explicit Services(DatabasePtr db);

    Json::Value registerUser(const Json::Value& input);
    Json::Value login(const Json::Value& input);
    Json::Value forgotPassword(const Json::Value& input);
    Json::Value resetPassword(const Json::Value& input);
    Json::Value currentUser(const drogon::HttpRequestPtr& request);
    void logout(const drogon::HttpRequestPtr& request);
    Json::Value updateProfile(const Json::Value& actor, const Json::Value& input);
    Json::Value inviteUser(const Json::Value& actor, const Json::Value& input);

    Json::Value appBootstrap(const Json::Value& actor);
    Json::Value listUsers();
    Json::Value listSquads();
    Json::Value listPracticeTemplates();

    Json::Value createPracticeTemplate(const Json::Value& actor, const Json::Value& input);
    Json::Value generateWeek(const Json::Value& actor, int templateId, const Json::Value& input);
    Json::Value weekOverview(const std::string& weekStart);
    Json::Value practiceDetail(const Json::Value& actor, int practiceId);
    Json::Value createPractice(const Json::Value& actor, const Json::Value& input);
    Json::Value deletePractice(const Json::Value& actor, int practiceId);
    Json::Value setAvailability(const Json::Value& actor, const Json::Value& input);

    Json::Value createLineup(const Json::Value& actor, const Json::Value& input);
    Json::Value substituteLineupSeat(const Json::Value& actor, int lineupId, const Json::Value& input);
    Json::Value requestSubstitution(const Json::Value& actor, const Json::Value& input);
    Json::Value respondToSubstitution(const Json::Value& actor, int requestId, const Json::Value& input);
    Json::Value weeklyLineupBoard(const std::string& weekStart);

    Json::Value recordAttendance(const Json::Value& actor, const Json::Value& input);
    Json::Value attendanceReport(const Json::Value& actor, const drogon::HttpRequestPtr& request);
    Json::Value assignMakeup(const Json::Value& actor, const Json::Value& input);
    Json::Value updateMakeup(const Json::Value& actor, int makeupId, const Json::Value& input);

    Json::Value createWorkout(const Json::Value& actor, const Json::Value& input);
    Json::Value workoutSummary(const Json::Value& actor, const drogon::HttpRequestPtr& request);

private:
    Json::Value requireActor(const Json::Value& actor);
    void requireRole(const Json::Value& actor, const std::vector<std::string>& roles);
    Json::Value findUser(int id);
    Json::Value practiceWithChildren(int practiceId);
    Json::Value practiceForActor(int practiceId, const Json::Value& actor);
    Json::Value lineupsForPractice(int practiceId);
    Json::Value athletesForPractice(int practiceId);

    DatabasePtr db_;
};

}
