#include "crewgle/Schema.h"
#include "crewgle/Security.h"

#include <array>
#include <string>
#include <vector>

namespace crewgle {

void migrate(Database& db) {
    db.exec(R"sql(
        CREATE TABLE IF NOT EXISTS squads (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            level TEXT NOT NULL DEFAULT 'Open'
        );

        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL CHECK(role IN ('Admin', 'Coach', 'Officer', 'Captain', 'Coxswain', 'Athlete')),
            side TEXT,
            class_year TEXT,
            phone TEXT,
            instagram_url TEXT,
            status TEXT NOT NULL DEFAULT 'active',
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS user_squads (
            user_id INTEGER NOT NULL,
            squad_id INTEGER NOT NULL,
            PRIMARY KEY (user_id, squad_id),
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (squad_id) REFERENCES squads(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            token TEXT NOT NULL UNIQUE,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            expires_at TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS practice_templates (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            squad_id INTEGER NOT NULL,
            created_by_user_id INTEGER NOT NULL,
            active INTEGER NOT NULL DEFAULT 1,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (squad_id) REFERENCES squads(id),
            FOREIGN KEY (created_by_user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS practice_template_slots (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            template_id INTEGER NOT NULL,
            weekday INTEGER NOT NULL CHECK(weekday BETWEEN 0 AND 6),
            title TEXT NOT NULL,
            practice_type TEXT NOT NULL CHECK(practice_type IN ('water', 'erg', 'lift', 'meeting')),
            start_time TEXT NOT NULL,
            end_time TEXT,
            location TEXT,
            notes TEXT,
            FOREIGN KEY (template_id) REFERENCES practice_templates(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS practices (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            template_id INTEGER,
            squad_id INTEGER NOT NULL,
            title TEXT NOT NULL,
            practice_type TEXT NOT NULL CHECK(practice_type IN ('water', 'erg', 'lift', 'meeting')),
            practice_date TEXT NOT NULL,
            week_start TEXT NOT NULL,
            start_time TEXT NOT NULL,
            end_time TEXT,
            location TEXT,
            notes TEXT,
            status TEXT NOT NULL DEFAULT 'final' CHECK(status IN ('draft', 'final', 'canceled')),
            FOREIGN KEY (template_id) REFERENCES practice_templates(id),
            FOREIGN KEY (squad_id) REFERENCES squads(id),
            UNIQUE(squad_id, practice_date, start_time, title)
        );

        CREATE TABLE IF NOT EXISTS availability (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            practice_id INTEGER NOT NULL,
            status TEXT NOT NULL CHECK(status IN ('available', 'land_only', 'maybe', 'out')),
            notes TEXT,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (practice_id) REFERENCES practices(id) ON DELETE CASCADE,
            UNIQUE(user_id, practice_id)
        );

        CREATE TABLE IF NOT EXISTS lineups (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            practice_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            boat_type TEXT,
            shell TEXT,
            oars TEXT,
            status TEXT NOT NULL DEFAULT 'published' CHECK(status IN ('draft', 'published')),
            created_by_user_id INTEGER NOT NULL,
            notes TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (practice_id) REFERENCES practices(id) ON DELETE CASCADE,
            FOREIGN KEY (created_by_user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS lineup_entries (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            lineup_id INTEGER NOT NULL,
            seat_number INTEGER NOT NULL,
            role_label TEXT,
            user_id INTEGER NOT NULL,
            FOREIGN KEY (lineup_id) REFERENCES lineups(id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES users(id),
            UNIQUE(lineup_id, seat_number),
            UNIQUE(lineup_id, user_id)
        );

        CREATE TABLE IF NOT EXISTS lineup_substitutions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            lineup_id INTEGER NOT NULL,
            seat_number INTEGER NOT NULL,
            original_user_id INTEGER,
            new_user_id INTEGER NOT NULL,
            reason TEXT,
            created_by_user_id INTEGER NOT NULL,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (lineup_id) REFERENCES lineups(id) ON DELETE CASCADE,
            FOREIGN KEY (original_user_id) REFERENCES users(id),
            FOREIGN KEY (new_user_id) REFERENCES users(id),
            FOREIGN KEY (created_by_user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS attendance (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            practice_id INTEGER NOT NULL,
            status TEXT NOT NULL CHECK(status IN ('present', 'absent', 'excused')),
            notes TEXT,
            recorded_by_user_id INTEGER NOT NULL,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (practice_id) REFERENCES practices(id) ON DELETE CASCADE,
            FOREIGN KEY (recorded_by_user_id) REFERENCES users(id),
            UNIQUE(user_id, practice_id)
        );

        CREATE TABLE IF NOT EXISTS makeup_workouts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            practice_id INTEGER,
            assigned_by_user_id INTEGER NOT NULL,
            title TEXT NOT NULL,
            description TEXT,
            due_date TEXT,
            status TEXT NOT NULL DEFAULT 'assigned' CHECK(status IN ('assigned', 'submitted', 'accepted', 'waived')),
            completion_notes TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (practice_id) REFERENCES practices(id) ON DELETE SET NULL,
            FOREIGN KEY (assigned_by_user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS erg_workouts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            workout_date TEXT NOT NULL,
            name TEXT NOT NULL,
            workout_type TEXT NOT NULL CHECK(workout_type IN ('UT3', 'UT2', 'UT1', 'ATR', 'VO2', 'ANP')),
            meters INTEGER NOT NULL,
            total_seconds INTEGER NOT NULL,
            split_seconds REAL NOT NULL,
            is_test INTEGER NOT NULL DEFAULT 0,
            test_piece TEXT,
            photo_url TEXT,
            notes TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS regattas (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            start_date TEXT NOT NULL,
            end_date TEXT,
            location TEXT,
            notes TEXT
        );

        CREATE TABLE IF NOT EXISTS regatta_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            regatta_id INTEGER NOT NULL,
            event_name TEXT NOT NULL,
            event_number TEXT,
            launch_time TEXT,
            race_time TEXT,
            lane TEXT,
            shell TEXT,
            oars TEXT,
            coach TEXT,
            notes TEXT,
            FOREIGN KEY (regatta_id) REFERENCES regattas(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS invitations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT 'Athlete',
            squad_id INTEGER,
            token TEXT NOT NULL UNIQUE,
            invited_by_user_id INTEGER NOT NULL,
            status TEXT NOT NULL DEFAULT 'sent' CHECK(status IN ('sent', 'accepted', 'revoked')),
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (squad_id) REFERENCES squads(id),
            FOREIGN KEY (invited_by_user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS substitution_requests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            lineup_id INTEGER NOT NULL,
            seat_number INTEGER NOT NULL,
            requester_user_id INTEGER NOT NULL,
            requested_sub_user_id INTEGER NOT NULL,
            status TEXT NOT NULL DEFAULT 'pending_sub' CHECK(status IN ('pending_sub', 'pending_coach', 'approved', 'denied')),
            requester_note TEXT,
            sub_response_note TEXT,
            coach_response_note TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (lineup_id) REFERENCES lineups(id) ON DELETE CASCADE,
            FOREIGN KEY (requester_user_id) REFERENCES users(id),
            FOREIGN KEY (requested_sub_user_id) REFERENCES users(id)
        );

        CREATE TABLE IF NOT EXISTS password_resets (
          id INTEGER PRIMARY KEY AUTOINCREMENT,
          user_id INTEGER NOT NULL,
          token_hash TEXT NOT NULL UNIQUE,
          expires_at TEXT NOT NULL,
          used_at TEXT,
          created_at TEXT DEFAULT CURRENT_TIMESTAMP,
          FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
        );
    )sql");
}

void seed(Database& db) {
    if (db.scalarInt("SELECT COUNT(*) FROM users") > 0) {
        return;
    }

    db.run("INSERT INTO squads (name, level) VALUES ('Open Men', 'Open'), ('Open Women', 'Open'), ('Novice Men', 'Novice'), ('Novice Women', 'Novice'), ('Coxswains', 'Support')");

    const std::string password = hashPassword("password");
    const std::vector<std::array<std::string, 8>> users = {{
        {"Owen Frazer", "coach@crewgle.test", "Coach", "", "", "555-0100",""},
        {"Evie Kohn", "evie@crewgle.test", "Coxswain", "", "Junior", "555-0101",""},
        {"Myra Patel", "myra@crewgle.test", "Coxswain", "", "Senior", "555-0102",""},
        {"Paul Bagley", "paul@crewgle.test", "Captain", "Star", "Senior", "555-0103",""},
        {"Will Gallegos", "will@crewgle.test", "Admin", "Star", "Junior", "555-0104",""},
        {"Tristan Hammit", "tristan@crewgle.test", "Athlete", "Port", "Junior", "555-0105",""},
        {"Cami Davis", "cami@crewgle.test", "Athlete", "Port", "Senior", "555-0106"},
        {"Landon Becker", "landon@crewgle.test", "Athlete", "Starboard", "Sophomore", "555-0107",""},
        {"Cameron Berg ", "cameron@crewgle.test", "Coach", "Port", "Sophomore", "555-0108",""},
        {"Frankie Andrade", "frankie@crewgle.test", "Coach", "Starboard", "Senior", "555-0109",""}
    }};

    for (int i = 0; i < static_cast<int>(users.size()); ++i) {
        const auto& user = users[i];
        db.run(
            "INSERT INTO users (name, email, password_hash, role, side, class_year, phone, instagram_url) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
            {user[0], user[1], password, user[2], user[3], user[4], user[5], user[6], user[7], std::string(""), std::string("")}
        );
        const int userId = db.lastInsertId();
        const int squadId = i == 3 ? 5 : (i == 7 || i == 8 ? 4 : 1);
        db.run("INSERT INTO user_squads (user_id, squad_id) VALUES (?, ?)", {userId, squadId});
    }

    db.run("INSERT INTO practice_templates (name, squad_id, created_by_user_id) VALUES ('Open Men Spring Week', 1, 1)");
    const int templateId = db.lastInsertId();
    const std::vector<std::array<std::string, 7>> slots = {{
        {"0", "Monday AM Water", "water", "06:00", "08:00", "Lake Bryan", "Technique row. If unavailable for water, find a sub 24 hours in advance."},
        {"1", "Tuesday AM BVB", "erg", "06:00", "07:00", "Brazos Valley Barbell", "UT2 land training and mobility."},
        {"2", "Wednesday AM Water", "water", "06:00", "08:00", "Lake Bryan", "Rate ladders and starts."},
        {"4", "Friday AT Pieces", "erg", "17:30", "19:00", "Erg Room", "AT intervals. Submit erg screens after practice."},
        {"5", "Saturday Lineups", "water", "07:00", "09:15", "Boathouse", "Seat racing and lineup testing."}
    }};
    for (const auto& slot : slots) {
        db.run(
            "INSERT INTO practice_template_slots (template_id, weekday, title, practice_type, start_time, end_time, location, notes) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
            {templateId, std::stoi(slot[0]), slot[1], slot[2], slot[3], slot[4], slot[5], slot[6]}
        );
    }

    db.run(R"sql(
        INSERT OR IGNORE INTO practices (template_id, squad_id, title, practice_type, practice_date, week_start, start_time, end_time, location, notes)
        SELECT template_id, 1, title, practice_type, date('2026-06-08', '+' || weekday || ' days'), '2026-06-08', start_time, end_time, location, notes
        FROM practice_template_slots WHERE template_id = ?
    )sql", {templateId});

    Json::Value practices = db.query("SELECT id FROM practices");
    Json::Value squadUsers = db.query("SELECT user_id FROM user_squads WHERE squad_id IN (1, 5)");
    for (const auto& practice : practices) {
        for (const auto& user : squadUsers) {
            const int userId = user["user_id"].asInt();
            const std::string status = userId == 6 ? "out" : (userId == 3 ? "land_only" : (userId == 10 ? "maybe" : "available"));
            db.run("INSERT INTO availability (user_id, practice_id, status, notes) VALUES (?, ?, ?, ?)",
                   {userId, practice["id"].asInt(), status, userId == 3 ? std::string("8 AM class, land only if water runs long") : std::string("")});
        }
    }

    db.run("INSERT INTO lineups (practice_id, name, boat_type, shell, oars, created_by_user_id, notes) VALUES (1, 'Wrecking Crew', '4+', 'Reveille', 'M1V', 1, 'Published test lineup')");
    const int lineupId = db.lastInsertId();
    const std::vector<std::array<int, 2>> entries = {{{1, 5}, {2, 7}, {3, 10}, {4, 6}, {5, 4}}};
    for (const auto& entry : entries) {
        db.run("INSERT INTO lineup_entries (lineup_id, seat_number, role_label, user_id) VALUES (?, ?, ?, ?)",
               {lineupId, entry[0], entry[0] == 5 ? std::string("Cox") : std::string("Seat"), entry[1]});
    }

    for (int userId : {1, 3, 4, 5, 7, 10}) {
        db.run("INSERT INTO attendance (user_id, practice_id, status, notes, recorded_by_user_id) VALUES (?, 1, ?, ?, 1)",
               {userId, userId == 10 ? std::string("excused") : std::string("present"), userId == 10 ? std::string("Academic conflict") : std::string("")});
    }
    db.run("INSERT INTO attendance (user_id, practice_id, status, notes, recorded_by_user_id) VALUES (6, 1, 'absent', 'No availability update', 1)");
    db.run("INSERT INTO makeup_workouts (user_id, practice_id, assigned_by_user_id, title, description, due_date) VALUES (6, 1, 1, 'Missed water makeup', '45 minutes UT2 plus mobility circuit.', '2026-06-15')");

    db.run("INSERT INTO erg_workouts (user_id, workout_date, name, workout_type, meters, total_seconds, split_seconds, is_test, test_piece, photo_url, notes) VALUES (5, '2026-06-03', '2k baseline', 'VO2', 2000, 438, 109.5, 1, '2k', '', 'First test of summer')");
    db.run("INSERT INTO erg_workouts (user_id, workout_date, name, workout_type, meters, total_seconds, split_seconds, is_test, test_piece, photo_url, notes) VALUES (5, '2026-06-10', '2k retest', 'VO2', 2000, 432, 108.0, 1, '2k', '', 'Cleaner middle thousand')");
    db.run("INSERT INTO erg_workouts (user_id, workout_date, name, workout_type, meters, total_seconds, split_seconds, is_test, test_piece, photo_url, notes) VALUES (7, '2026-06-09', 'Steady state', 'UT2', 12000, 3180, 132.5, 0, '', '', '')");

    db.run("INSERT INTO regattas (name, start_date, end_date, location, notes) VALUES ('Heart of Texas', '2026-04-18', '2026-04-19', 'Austin, TX', 'Launch chute times and shell assignments imported from the spreadsheet workflow.')");
    const int regattaId = db.lastInsertId();
    db.run("INSERT INTO regatta_events (regatta_id, event_name, event_number, launch_time, race_time, lane, shell, oars, coach) VALUES (?, 'M Varsity 4+', '33', '12:18', '12:48', 'A Lane 2', 'Cheyenne', 'M1V', 'Alex Morgan')", {regattaId});
    db.run("INSERT INTO regatta_events (regatta_id, event_name, event_number, launch_time, race_time, lane, shell, oars, coach) VALUES (?, 'M Novice 8+', '31', '15:30', '16:00', 'A Lane 4', 'Wrecking Crew', 'M2V', 'Maya Ortiz')", {regattaId});
}

}
