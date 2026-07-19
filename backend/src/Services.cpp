#include "crewgle/Services.h"

#include "crewgle/Http.h"
#include "crewgle/Security.h"
#include "crewgle/Email.h"

#include <stdexcept>
#include <utility>

namespace crewgle {

Services::Services(DatabasePtr db) : db_(std::move(db)) {}

Json::Value Services::requireActor(const Json::Value& actor) {
    if (actor.isNull()) {
        throw std::invalid_argument("Authentication is required.");
    }
    return actor;
}

void Services::requireRole(const Json::Value& actor, const std::vector<std::string>& roles) {
    requireActor(actor);
    if (!roleIn(actor, roles)) {
        throw std::invalid_argument("You do not have permission for that action.");
    }
}

Json::Value Services::findUser(int id) {
    Json::Value user = db_->one("SELECT id, name, email, role, side, class_year, phone, instagram_url, status FROM users WHERE id = ?", {id});
    if (user.isNull()) {
        throw std::out_of_range("User not found.");
    }
    return user;
}

Json::Value Services::registerUser(const Json::Value& input) {
    requireFields(input, {"name", "email", "password", "role"});
    db_->run(
        "INSERT INTO users (name, email, password_hash, role, side, class_year, phone, instagram_url) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        {input["name"].asString(), input["email"].asString(), hashPassword(input["password"].asString()), input["role"].asString(),
         optionalText(input, "side"), optionalText(input, "class_year"), optionalText(input, "phone"), optionalText(input, "instagram_url")}
    );
    const int userId = db_->lastInsertId();
    db_->run("INSERT OR IGNORE INTO user_squads (user_id, squad_id) VALUES (?, ?)", {userId, input.isMember("squad_id") ? input["squad_id"].asInt() : 1});

    Json::Value response;
    response["user"] = findUser(userId);
    return response;
}

Json::Value Services::login(const Json::Value& input) {
    requireFields(input, {"email", "password"});
    Json::Value user = db_->one(
        "SELECT id, name, email, role, side, class_year, phone, instagram_url, status "
        "FROM users WHERE email = ? AND password_hash = ?",
        {input["email"].asString(), hashPassword(input["password"].asString())}
    );
    if (user.isNull()) {
        throw std::invalid_argument("Invalid email or password.");
    }

    const std::string token = makeToken();
    db_->run("INSERT INTO sessions (user_id, token, expires_at) VALUES (?, ?, datetime('now', '+14 days'))", {user["id"].asInt(), token});

    Json::Value response;
    response["token"] = token;
    response["user"] = user;
    return response;
}

Json::Value Services::forgotPassword(const Json::Value& input) {
    requireFields(input, {"email"});

    const std::string email = input["email"].asString();

    Json::Value response;
    response["message"] = "If that email exists, a reset link has been sent.";

    Json::Value user = db_->one(
        "SELECT id, email FROM users WHERE email = ?",
        {email}
    );

    if (!user.isNull()) {
        const std::string token = makeToken();
        const std::string tokenHash = hashPassword(token);

        db_->run(
            "INSERT INTO password_resets (user_id, token_hash, expires_at) "
            "VALUES (?, ?, datetime('now', '+30 minutes'))",
            {user["id"].asInt(), tokenHash}
        );

        const std::string resetLink =
            "http://localhost:5173/reset-password?token=" + token;

        Email::sendPasswordResetEmail(email, resetLink);
    }

    return response;
}
Json::Value Services::resetPassword(const Json::Value& input) {
    requireFields(input, {"token", "password"});

    const std::string& tokenHash = hashPassword(input["token"].asString());

    Json::Value reset = db_->one(
        "SELECT id, user_id FROM password_resets "
        "WHERE token_hash = ? "
        "AND used_at IS NULL "
        "AND expires_at > datetime('now')",
        {tokenHash}
    );

    if (reset.isNull()) {
        throw std::invalid_argument("Invalid or expired reset link.");
    }

    db_->run(
        "UPDATE users SET password_hash = ? WHERE id = ?",
        {hashPassword(input["password"].asString()), reset["user_id"].asInt()}
    );

    db_->run(
        "UPDATE password_resets SET used_at = datetime('now') WHERE id = ?",
        {reset["id"].asInt()}
    );

    Json::Value response;
    response["message"] = "Password updated.";
    return response;
}

Json::Value Services::currentUser(const drogon::HttpRequestPtr& request) {
    const std::string token = bearerToken(request);
    if (token.empty()) {
        return {};
    }
    Json::Value user = db_->one(
        "SELECT u.id, u.name, u.email, u.role, u.side, u.class_year, u.phone, u.instagram_url, u.status "
        "FROM sessions s JOIN users u ON u.id = s.user_id "
        "WHERE s.token = ? AND s.expires_at > datetime('now')",
        {token}
    );
    if (!user.isNull()) {
        user["squads"] = db_->query(
            "SELECT s.id, s.name, s.level FROM user_squads us JOIN squads s ON s.id = us.squad_id WHERE us.user_id = ? ORDER BY s.name",
            {user["id"].asInt()}
        );
    }
    return user;
}

void Services::logout(const drogon::HttpRequestPtr& request) {
    const std::string token = bearerToken(request);
    if (!token.empty()) {
        db_->run("DELETE FROM sessions WHERE token = ?", {token});
    }
}

Json::Value Services::updateProfile(const Json::Value& actor, const Json::Value& input) {
    requireActor(actor);
    const int userId = input.isMember("user_id") ? input["user_id"].asInt() : actor["id"].asInt();
    if (userId != actor["id"].asInt()) {
        requireRole(actor, {"Admin"});
    }
    db_->run(
        "UPDATE users SET phone = ?, side = ?, class_year = ?, instagram_url = ? WHERE id = ?",
        {optionalText(input, "phone", actor.get("phone", "").asString()),
         optionalText(input, "side", actor.get("side", "").asString()),
         optionalText(input, "class_year", actor.get("class_year", "").asString()),
         optionalText(input, "instagram_url", actor.get("instagram_url", "").asString()),
         userId}
    );
    Json::Value response;
    response["user"] = findUser(userId);
    return response;
}

Json::Value Services::inviteUser(const Json::Value& actor, const Json::Value& input) {
    requireRole(actor, {"Admin", "Coach", "Officer"});
    requireFields(input, {"email"});
    const std::string token = makeToken();
    db_->run(
        "INSERT INTO invitations (email, role, squad_id, token, invited_by_user_id) VALUES (?, ?, ?, ?, ?)",
        {input["email"].asString(), optionalText(input, "role", "Athlete"), input.isMember("squad_id") ? input["squad_id"].asInt() : 1, token, actor["id"].asInt()}
    );
    Json::Value response;
    response["status"] = "sent";
    response["email"] = input["email"].asString();
    response["token"] = token;
    response["message"] = "Email delivery is stubbed for this PoC; this token represents the registration invite.";
    return response;
}

Json::Value Services::listUsers() {
    return db_->query(R"sql(
        SELECT u.id, u.name, u.email, u.role, u.side, u.class_year, u.phone, u.instagram_url, u.status,
        COALESCE(group_concat(s.name, ', '), '') AS squads
        FROM users u
        LEFT JOIN user_squads us ON us.user_id = u.id
        LEFT JOIN squads s ON s.id = us.squad_id
        GROUP BY u.id
        ORDER BY CASE u.role WHEN 'Coach' THEN 0 WHEN 'Officer' THEN 1 WHEN 'Captain' THEN 2 WHEN 'Coxswain' THEN 3 ELSE 4 END, u.name
    )sql");
}

Json::Value Services::listSquads() {
    return db_->query("SELECT id, name, level FROM squads ORDER BY name");
}

Json::Value Services::appBootstrap(const Json::Value& actor) {
    requireActor(actor);
    Json::Value response;
    response["user"] = actor;
    response["users"] = listUsers();
    response["squads"] = listSquads();
    response["templates"] = listPracticeTemplates();
    response["current_week"] = weekOverview("2026-06-08");
    response["lineup_board"] = weeklyLineupBoard("2026-06-08");
    response["attendance"] = attendanceReport(actor, nullptr);
    response["workouts"] = workoutSummary(actor, nullptr);
    return response;
}

Json::Value Services::listPracticeTemplates() {
    Json::Value templates = db_->query(R"sql(
        SELECT pt.id, pt.name, pt.squad_id, s.name AS squad_name, pt.created_by_user_id, u.name AS created_by_name, pt.active, pt.created_at
        FROM practice_templates pt
        JOIN squads s ON s.id = pt.squad_id
        JOIN users u ON u.id = pt.created_by_user_id
        ORDER BY pt.name
    )sql");
    for (Json::ArrayIndex i = 0; i < templates.size(); ++i) {
        templates[i]["slots"] = db_->query(
            "SELECT id, weekday, title, practice_type, start_time, end_time, location, notes FROM practice_template_slots WHERE template_id = ? ORDER BY weekday, start_time",
            {templates[i]["id"].asInt()}
        );
    }
    return templates;
}

Json::Value Services::createPracticeTemplate(const Json::Value& actor, const Json::Value& input) {
    requireRole(actor, {"Coach", "Officer"});
    requireFields(input, {"name", "squad_id", "slots"});

    db_->exec("BEGIN");
    try {
        db_->run("INSERT INTO practice_templates (name, squad_id, created_by_user_id) VALUES (?, ?, ?)",
                 {input["name"].asString(), input["squad_id"].asInt(), actor["id"].asInt()});
        const int templateId = db_->lastInsertId();
        for (const auto& slot : input["slots"]) {
            requireFields(slot, {"weekday", "title", "practice_type", "start_time"});
            db_->run(
                "INSERT INTO practice_template_slots (template_id, weekday, title, practice_type, start_time, end_time, location, notes) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                {templateId, slot["weekday"].asInt(), slot["title"].asString(), slot["practice_type"].asString(), slot["start_time"].asString(),
                 optionalText(slot, "end_time"), optionalText(slot, "location"), optionalText(slot, "notes")}
            );
        }
        db_->exec("COMMIT");
    } catch (...) {
        db_->exec("ROLLBACK");
        throw;
    }

    Json::Value response;
    response["templates"] = listPracticeTemplates();
    return response;
}

Json::Value Services::generateWeek(const Json::Value& actor, int templateId, const Json::Value& input) {
    requireRole(actor, {"Coach", "Officer"});
    requireFields(input, {"week_start"});

    Json::Value tmpl = db_->one("SELECT id, squad_id FROM practice_templates WHERE id = ?", {templateId});
    if (tmpl.isNull()) {
        throw std::out_of_range("Practice template not found.");
    }

    db_->run(R"sql(
        INSERT OR IGNORE INTO practices (template_id, squad_id, title, practice_type, practice_date, week_start, start_time, end_time, location, notes, status)
        SELECT template_id, ?, title, practice_type, date(?, '+' || weekday || ' days'), ?, start_time, end_time, location, notes, 'final'
        FROM practice_template_slots WHERE template_id = ?
    )sql", {tmpl["squad_id"].asInt(), input["week_start"].asString(), input["week_start"].asString(), templateId});

    Json::Value practices = db_->query("SELECT id, squad_id FROM practices WHERE week_start = ?", {input["week_start"].asString()});
    for (const auto& practice : practices) {
        Json::Value users = db_->query("SELECT user_id FROM user_squads WHERE squad_id = ? OR squad_id = 5", {practice["squad_id"].asInt()});
        for (const auto& user : users) {
            db_->run("INSERT OR IGNORE INTO availability (user_id, practice_id, status, notes) VALUES (?, ?, 'maybe', '')",
                     {user["user_id"].asInt(), practice["id"].asInt()});
        }
    }
    return weekOverview(input["week_start"].asString());
}

Json::Value Services::weekOverview(const std::string& weekStart) {
    Json::Value response;
    response["week_start"] = weekStart;
    response["practices"] = db_->query(R"sql(
        SELECT p.*, s.name AS squad_name,
        (SELECT COUNT(*) FROM availability a WHERE a.practice_id = p.id AND a.status = 'available') AS available_count,
        (SELECT COUNT(*) FROM availability a WHERE a.practice_id = p.id AND a.status = 'land_only') AS land_only_count,
        (SELECT COUNT(*) FROM availability a WHERE a.practice_id = p.id AND a.status = 'maybe') AS maybe_count,
        (SELECT COUNT(*) FROM availability a WHERE a.practice_id = p.id AND a.status = 'out') AS out_count,
        (SELECT COUNT(*) FROM attendance t WHERE t.practice_id = p.id AND t.status = 'present') AS present_count,
        (SELECT COUNT(*) FROM lineups l WHERE l.practice_id = p.id) AS lineup_count
        FROM practices p JOIN squads s ON s.id = p.squad_id
        WHERE p.week_start = ? ORDER BY p.practice_date, p.start_time
    )sql", {weekStart});
    response["makeups_due"] = db_->query(R"sql(
        SELECT m.*, u.name AS user_name, p.title AS practice_title
        FROM makeup_workouts m JOIN users u ON u.id = m.user_id LEFT JOIN practices p ON p.id = m.practice_id
        WHERE m.status IN ('assigned', 'submitted') ORDER BY m.due_date, u.name
    )sql");
    response["regattas"] = db_->query("SELECT * FROM regattas ORDER BY start_date");
    return response;
}

Json::Value Services::lineupsForPractice(int practiceId) {
    Json::Value lineups = db_->query("SELECT * FROM lineups WHERE practice_id = ? ORDER BY id", {practiceId});
    for (Json::ArrayIndex i = 0; i < lineups.size(); ++i) {
        lineups[i]["entries"] = db_->query(R"sql(
            SELECT le.id, le.seat_number, le.role_label, le.user_id, u.name AS user_name, u.role, u.side,
            COALESCE(a.status, 'maybe') AS availability_status
            FROM lineup_entries le JOIN users u ON u.id = le.user_id
            LEFT JOIN availability a ON a.user_id = le.user_id AND a.practice_id = ?
            WHERE le.lineup_id = ? ORDER BY le.seat_number
        )sql", {practiceId, lineups[i]["id"].asInt()});
        lineups[i]["substitutions"] = db_->query(R"sql(
            SELECT ls.*, ou.name AS original_user_name, nu.name AS new_user_name, cu.name AS created_by_name
            FROM lineup_substitutions ls
            LEFT JOIN users ou ON ou.id = ls.original_user_id
            JOIN users nu ON nu.id = ls.new_user_id
            JOIN users cu ON cu.id = ls.created_by_user_id
            WHERE ls.lineup_id = ? ORDER BY ls.created_at
        )sql", {lineups[i]["id"].asInt()});
    }
    return lineups;
}

Json::Value Services::athletesForPractice(int practiceId) {
    return db_->query(R"sql(
        SELECT u.id, u.name, u.role, u.side, u.class_year, COALESCE(a.status, 'maybe') AS availability_status,
        COALESCE(a.notes, '') AS availability_notes,
        (SELECT COUNT(*) FROM attendance at JOIN practices p2 ON p2.id = at.practice_id
         WHERE at.user_id = u.id AND at.status = 'present' AND p2.week_start = p.week_start AND p2.practice_type = 'water') AS water_count,
        (SELECT COUNT(*) FROM attendance at JOIN practices p2 ON p2.id = at.practice_id
         WHERE at.user_id = u.id AND at.status = 'present' AND p2.week_start = p.week_start AND p2.practice_type != 'water') AS land_count
        FROM practices p
        JOIN user_squads us ON us.squad_id = p.squad_id OR us.squad_id = 5
        JOIN users u ON u.id = us.user_id
        LEFT JOIN availability a ON a.user_id = u.id AND a.practice_id = p.id
        WHERE p.id = ?
        GROUP BY u.id
        ORDER BY CASE u.role WHEN 'Coach' THEN 0 WHEN 'Coxswain' THEN 1 ELSE 2 END, availability_status, u.name
    )sql", {practiceId});
}

Json::Value Services::practiceWithChildren(int practiceId) {
    Json::Value practice = db_->one(R"sql(
        SELECT p.*, s.name AS squad_name,
        (SELECT COUNT(*) FROM availability a WHERE a.practice_id = p.id AND a.status = 'available') AS available_count,
        (SELECT COUNT(*) FROM availability a WHERE a.practice_id = p.id AND a.status = 'land_only') AS land_only_count,
        (SELECT COUNT(*) FROM availability a WHERE a.practice_id = p.id AND a.status = 'maybe') AS maybe_count,
        (SELECT COUNT(*) FROM availability a WHERE a.practice_id = p.id AND a.status = 'out') AS out_count,
        (SELECT COUNT(*) FROM attendance t WHERE t.practice_id = p.id AND t.status = 'present') AS present_count,
        (SELECT COUNT(*) FROM lineups l WHERE l.practice_id = p.id) AS lineup_count
        FROM practices p JOIN squads s ON s.id = p.squad_id WHERE p.id = ?
    )sql", {practiceId});
    if (practice.isNull()) {
        throw std::out_of_range("Practice not found.");
    }
    practice["athletes"] = athletesForPractice(practiceId);
    practice["attendance"] = db_->query(R"sql(
        SELECT at.*, u.name AS user_name, u.role
        FROM attendance at JOIN users u ON u.id = at.user_id
        WHERE at.practice_id = ? ORDER BY u.name
    )sql", {practiceId});
    practice["lineups"] = lineupsForPractice(practiceId);
    return practice;
}

Json::Value Services::practiceForActor(int practiceId, const Json::Value& actor) {
    Json::Value practice = practiceWithChildren(practiceId);
    if (roleIn(actor, {"Coach"})) {
        return practice;
    }

    Json::Value athletes(Json::arrayValue);
    for (const auto& athlete : practice["athletes"]) {
        if (athlete["id"].asInt() == actor["id"].asInt()) {
            athletes.append(athlete);
        }
    }
    practice["athletes"] = athletes;

    Json::Value attendance(Json::arrayValue);
    for (const auto& record : practice["attendance"]) {
        if (record.isMember("user_id") && record["user_id"].asInt() == actor["id"].asInt()) {
            attendance.append(record);
        }
    }
    practice["attendance"] = attendance;
    return practice;
}

Json::Value Services::practiceDetail(const Json::Value& actor, int practiceId) {
    return practiceForActor(practiceId, actor);
}

Json::Value Services::createPractice(const Json::Value& actor, const Json::Value& input) {
    requireRole(actor, {"Coach", "Officer"});
    requireFields(input, {"squad_id", "title", "practice_type", "practice_date", "week_start", "start_time"});
    db_->run(
        "INSERT OR IGNORE INTO practices (squad_id, title, practice_type, practice_date, week_start, start_time, end_time, location, notes, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        {input["squad_id"].asInt(), input["title"].asString(), input["practice_type"].asString(), input["practice_date"].asString(), input["week_start"].asString(),
         input["start_time"].asString(), optionalText(input, "end_time"), optionalText(input, "location"), optionalText(input, "notes"), optionalText(input, "status", "final")}
    );
    const int practiceId = db_->lastInsertId();
    if (practiceId > 0) {
        Json::Value users = db_->query("SELECT user_id FROM user_squads WHERE squad_id = ? OR squad_id = 5", {input["squad_id"].asInt()});
        for (const auto& user : users) {
            db_->run("INSERT OR IGNORE INTO availability (user_id, practice_id, status, notes) VALUES (?, ?, 'maybe', '')", {user["user_id"].asInt(), practiceId});
        }
    }
    Json::Value response;
    response["week"] = weekOverview(input["week_start"].asString());
    response["practice"] = practiceId > 0 ? practiceWithChildren(practiceId) : Json::Value();
    return response;
}

Json::Value Services::deletePractice(const Json::Value& actor, int practiceId) {
    requireRole(actor, {"Coach", "Officer"});
    Json::Value practice = db_->one("SELECT id, week_start FROM practices WHERE id = ?", {practiceId});
    if (practice.isNull()) {
        throw std::out_of_range("Practice not found.");
    }
    db_->run("DELETE FROM practices WHERE id = ?", {practiceId});

    Json::Value response;
    response["status"] = "deleted";
    response["week"] = weekOverview(practice["week_start"].asString());
    return response;
}

Json::Value Services::setAvailability(const Json::Value& actor, const Json::Value& input) {
    requireActor(actor);
    requireFields(input, {"practice_id", "status"});
    const int userId = input.isMember("user_id") ? input["user_id"].asInt() : actor["id"].asInt();
    if (userId != actor["id"].asInt()) {
        requireRole(actor, {"Coach"});
    }
    db_->run(R"sql(
        INSERT INTO availability (user_id, practice_id, status, notes) VALUES (?, ?, ?, ?)
        ON CONFLICT(user_id, practice_id) DO UPDATE SET status = excluded.status, notes = excluded.notes, updated_at = CURRENT_TIMESTAMP
    )sql", {userId, input["practice_id"].asInt(), input["status"].asString(), optionalText(input, "notes")});
    return practiceForActor(input["practice_id"].asInt(), actor);
}

Json::Value Services::createLineup(const Json::Value& actor, const Json::Value& input) {
    requireRole(actor, {"Coach", "Officer"});
    requireFields(input, {"practice_id", "name", "entries"});
    db_->exec("BEGIN");
    try {
        db_->run(
            "INSERT INTO lineups (practice_id, name, boat_type, shell, oars, status, created_by_user_id, notes) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
            {input["practice_id"].asInt(), input["name"].asString(), optionalText(input, "boat_type"), optionalText(input, "shell"),
             optionalText(input, "oars"), optionalText(input, "status", "published"), actor["id"].asInt(), optionalText(input, "notes")}
        );
        const int lineupId = db_->lastInsertId();
        for (const auto& entry : input["entries"]) {
            requireFields(entry, {"seat_number", "user_id"});
            db_->run("INSERT INTO lineup_entries (lineup_id, seat_number, role_label, user_id) VALUES (?, ?, ?, ?)",
                     {lineupId, entry["seat_number"].asInt(), optionalText(entry, "role_label", "Seat"), entry["user_id"].asInt()});
        }
        db_->exec("COMMIT");
    } catch (...) {
        db_->exec("ROLLBACK");
        throw;
    }
    return practiceWithChildren(input["practice_id"].asInt());
}

Json::Value Services::substituteLineupSeat(const Json::Value& actor, int lineupId, const Json::Value& input) {
    requireRole(actor, {"Coach", "Officer", "Captain"});
    requireFields(input, {"seat_number", "new_user_id"});
    Json::Value lineup = db_->one("SELECT id, practice_id FROM lineups WHERE id = ?", {lineupId});
    if (lineup.isNull()) {
        throw std::out_of_range("Lineup not found.");
    }

    Json::Value entry = db_->one("SELECT user_id FROM lineup_entries WHERE lineup_id = ? AND seat_number = ?", {lineupId, input["seat_number"].asInt()});
    const int originalUserId = entry.isNull() ? 0 : entry["user_id"].asInt();
    if (entry.isNull()) {
        db_->run("INSERT INTO lineup_entries (lineup_id, seat_number, role_label, user_id) VALUES (?, ?, 'Seat', ?)",
                 {lineupId, input["seat_number"].asInt(), input["new_user_id"].asInt()});
    } else {
        db_->run("UPDATE lineup_entries SET user_id = ? WHERE lineup_id = ? AND seat_number = ?",
                 {input["new_user_id"].asInt(), lineupId, input["seat_number"].asInt()});
    }
    db_->run("INSERT INTO lineup_substitutions (lineup_id, seat_number, original_user_id, new_user_id, reason, created_by_user_id) VALUES (?, ?, ?, ?, ?, ?)",
             {lineupId, input["seat_number"].asInt(), originalUserId, input["new_user_id"].asInt(), optionalText(input, "reason"), actor["id"].asInt()});
    return practiceWithChildren(lineup["practice_id"].asInt());
}

Json::Value Services::requestSubstitution(const Json::Value& actor, const Json::Value& input) {
    requireActor(actor);
    requireFields(input, {"lineup_id", "seat_number", "requested_sub_user_id"});
    Json::Value lineup = db_->one("SELECT id, practice_id FROM lineups WHERE id = ?", {input["lineup_id"].asInt()});
    if (lineup.isNull()) {
        throw std::out_of_range("Lineup not found.");
    }
    db_->run(
        "INSERT INTO substitution_requests (lineup_id, seat_number, requester_user_id, requested_sub_user_id, requester_note) VALUES (?, ?, ?, ?, ?)",
        {input["lineup_id"].asInt(), input["seat_number"].asInt(), actor["id"].asInt(), input["requested_sub_user_id"].asInt(), optionalText(input, "requester_note")}
    );
    Json::Value response;
    response["status"] = "pending_sub";
    response["message"] = "Email delivery is stubbed; in production this would email the requested substitute first, then coaches after substitute confirmation.";
    response["practice"] = practiceWithChildren(lineup["practice_id"].asInt());
    return response;
}

Json::Value Services::respondToSubstitution(const Json::Value& actor, int requestId, const Json::Value& input) {
    requireActor(actor);
    Json::Value request = db_->one("SELECT sr.*, l.practice_id FROM substitution_requests sr JOIN lineups l ON l.id = sr.lineup_id WHERE sr.id = ?", {requestId});
    if (request.isNull()) {
        throw std::out_of_range("Substitution request not found.");
    }

    const std::string decision = optionalText(input, "decision");
    if (request["status"].asString() == "pending_sub") {
        if (actor["id"].asInt() != request["requested_sub_user_id"].asInt()) {
            requireRole(actor, {"Coach", "Officer"});
        }
        const std::string next = decision == "confirm" ? "pending_coach" : "denied";
        db_->run("UPDATE substitution_requests SET status = ?, sub_response_note = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?",
                 {next, optionalText(input, "note"), requestId});
    } else if (request["status"].asString() == "pending_coach") {
        requireRole(actor, {"Coach", "Officer"});
        if (decision == "confirm") {
            Json::Value substitution;
            substitution["seat_number"] = request["seat_number"];
            substitution["new_user_id"] = request["requested_sub_user_id"];
            substitution["reason"] = "Approved substitution request";
            substituteLineupSeat(actor, request["lineup_id"].asInt(), substitution);
            db_->run("UPDATE substitution_requests SET status = 'approved', coach_response_note = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?",
                     {optionalText(input, "note"), requestId});
        } else {
            db_->run("UPDATE substitution_requests SET status = 'denied', coach_response_note = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?",
                     {optionalText(input, "note"), requestId});
        }
    }

    Json::Value response;
    response["practice"] = practiceWithChildren(request["practice_id"].asInt());
    return response;
}

Json::Value Services::weeklyLineupBoard(const std::string& weekStart) {
    Json::Value board;
    board["week_start"] = weekStart;
    board["practices"] = db_->query("SELECT * FROM practices WHERE week_start = ? ORDER BY practice_date, start_time", {weekStart});
    for (Json::ArrayIndex i = 0; i < board["practices"].size(); ++i) {
        board["practices"][i]["lineups"] = lineupsForPractice(board["practices"][i]["id"].asInt());
    }
    return board;
}

Json::Value Services::recordAttendance(const Json::Value& actor, const Json::Value& input) {
    requireRole(actor, {"Coach", "Officer", "Captain", "Coxswain"});
    requireFields(input, {"practice_id", "records"});
    for (const auto& record : input["records"]) {
        requireFields(record, {"user_id", "status"});
        db_->run(R"sql(
            INSERT INTO attendance (user_id, practice_id, status, notes, recorded_by_user_id) VALUES (?, ?, ?, ?, ?)
            ON CONFLICT(user_id, practice_id) DO UPDATE SET status = excluded.status, notes = excluded.notes,
            recorded_by_user_id = excluded.recorded_by_user_id, updated_at = CURRENT_TIMESTAMP
        )sql", {record["user_id"].asInt(), input["practice_id"].asInt(), record["status"].asString(), optionalText(record, "notes"), actor["id"].asInt()});
    }
    return practiceWithChildren(input["practice_id"].asInt());
}

Json::Value Services::attendanceReport(const Json::Value& actor, const drogon::HttpRequestPtr& request) {
    requireActor(actor);
    const bool canSeeAll = roleIn(actor, {"Coach", "Officer", "Captain"});
    const std::string userFilter = queryParam(request, "user_id");
    int userId = userFilter.empty() ? actor["id"].asInt() : std::stoi(userFilter);
    if (!canSeeAll) {
        userId = actor["id"].asInt();
    }
    const int sqlUser = canSeeAll && userFilter.empty() ? 0 : userId;

    Json::Value response;
    response["misses"] = db_->query(R"sql(
        SELECT at.*, u.name AS user_name, p.title AS practice_title, p.practice_date, p.practice_type
        FROM attendance at JOIN users u ON u.id = at.user_id JOIN practices p ON p.id = at.practice_id
        WHERE at.status IN ('absent', 'excused') AND (? = 0 OR at.user_id = ?)
        ORDER BY p.practice_date DESC, u.name
    )sql", {sqlUser, sqlUser});
    response["makeups"] = db_->query(R"sql(
        SELECT m.*, u.name AS user_name, p.title AS practice_title, p.practice_date
        FROM makeup_workouts m JOIN users u ON u.id = m.user_id LEFT JOIN practices p ON p.id = m.practice_id
        WHERE (? = 0 OR m.user_id = ?) ORDER BY m.due_date, u.name
    )sql", {sqlUser, sqlUser});
    return response;
}

Json::Value Services::assignMakeup(const Json::Value& actor, const Json::Value& input) {
    requireRole(actor, {"Coach", "Officer"});
    requireFields(input, {"user_id", "title"});
    if (input.isMember("practice_id") && !input["practice_id"].isNull() && input["practice_id"].asInt() > 0) {
        db_->run("INSERT INTO makeup_workouts (user_id, practice_id, assigned_by_user_id, title, description, due_date) VALUES (?, ?, ?, ?, ?, ?)",
                 {input["user_id"].asInt(), input["practice_id"].asInt(), actor["id"].asInt(), input["title"].asString(), optionalText(input, "description"), optionalText(input, "due_date")});
    } else {
        db_->run("INSERT INTO makeup_workouts (user_id, assigned_by_user_id, title, description, due_date) VALUES (?, ?, ?, ?, ?)",
                 {input["user_id"].asInt(), actor["id"].asInt(), input["title"].asString(), optionalText(input, "description"), optionalText(input, "due_date")});
    }
    return attendanceReport(actor, nullptr);
}

Json::Value Services::updateMakeup(const Json::Value& actor, int makeupId, const Json::Value& input) {
    requireActor(actor);
    Json::Value makeup = db_->one("SELECT * FROM makeup_workouts WHERE id = ?", {makeupId});
    if (makeup.isNull()) {
        throw std::out_of_range("Makeup workout not found.");
    }
    if (makeup["user_id"].asInt() != actor["id"].asInt()) {
        requireRole(actor, {"Coach", "Officer"});
    }
    db_->run("UPDATE makeup_workouts SET status = ?, completion_notes = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?",
             {optionalText(input, "status", makeup["status"].asString()), optionalText(input, "completion_notes"), makeupId});
    return attendanceReport(actor, nullptr);
}

Json::Value Services::createWorkout(const Json::Value& actor, const Json::Value& input) {
    requireActor(actor);
    requireFields(input, {"workout_date", "name", "workout_type", "meters", "total_seconds"});
    const int meters = input["meters"].asInt();
    const int totalSeconds = input["total_seconds"].asInt();
    if (meters <= 0 || totalSeconds <= 0) {
        throw std::invalid_argument("Meters and total_seconds must be positive.");
    }
    const double split = static_cast<double>(totalSeconds) / (static_cast<double>(meters) / 500.0);
    db_->run(R"sql(
        INSERT INTO erg_workouts (user_id, workout_date, name, workout_type, meters, total_seconds, split_seconds, is_test, test_piece, photo_url, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )sql", {actor["id"].asInt(), input["workout_date"].asString(), input["name"].asString(), input["workout_type"].asString(),
            meters, totalSeconds, split, input.isMember("is_test") && input["is_test"].asBool() ? 1 : 0,
            optionalText(input, "test_piece"), optionalText(input, "photo_url"), optionalText(input, "notes")});
    return workoutSummary(actor, nullptr);
}

Json::Value Services::workoutSummary(const Json::Value& actor, const drogon::HttpRequestPtr& request) {
    requireActor(actor);
    const bool canSeeAll = roleIn(actor, {"Coach", "Officer", "Captain"});
    const std::string requestedUser = queryParam(request, "user_id");
    int userId = requestedUser.empty() ? actor["id"].asInt() : std::stoi(requestedUser);
    if (!canSeeAll) {
        userId = actor["id"].asInt();
    }

    Json::Value response;
    response["user_id"] = userId;
    response["totals"] = db_->one(R"sql(
        SELECT COUNT(*) AS workout_count, COALESCE(SUM(meters), 0) AS meters,
        COALESCE(AVG(split_seconds), 0) AS avg_split_seconds
        FROM erg_workouts WHERE user_id = ?
    )sql", {userId});
    response["by_type"] = db_->query("SELECT workout_type, COUNT(*) AS count, SUM(meters) AS meters FROM erg_workouts WHERE user_id = ? GROUP BY workout_type ORDER BY workout_type", {userId});
    response["test_progress"] = db_->query(R"sql(
        SELECT test_piece, workout_date, meters, total_seconds, split_seconds
        FROM erg_workouts WHERE user_id = ? AND is_test = 1 ORDER BY test_piece, workout_date
    )sql", {userId});
    response["recent"] = db_->query("SELECT * FROM erg_workouts WHERE user_id = ? ORDER BY workout_date DESC, id DESC LIMIT 12", {userId});
    response["leaderboard"] = db_->query(R"sql(
        SELECT u.name, u.squads, ew.test_piece, MIN(ew.split_seconds) AS best_split_seconds
        FROM (
            SELECT u.id, u.name, COALESCE(group_concat(s.name, ', '), '') AS squads
            FROM users u LEFT JOIN user_squads us ON us.user_id = u.id LEFT JOIN squads s ON s.id = us.squad_id
            GROUP BY u.id
        ) u
        JOIN erg_workouts ew ON ew.user_id = u.id
        WHERE ew.is_test = 1
        GROUP BY u.id, ew.test_piece
        ORDER BY ew.test_piece, best_split_seconds
    )sql");
    return response;
}

}
