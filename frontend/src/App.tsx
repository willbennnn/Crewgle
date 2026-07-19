import {
  Activity,
  CalendarDays,
  CalendarPlus,
  Check,
  ClipboardList,
  Download,
  Dumbbell,
  Flag,
  Link as LinkIcon,
  LogOut,
  Mail,
  Menu,
  Phone,
  Plus,
  RefreshCw,
  Settings,
  ShipWheel,
  Trash2,
  UserPlus,
  Users,
  X,
} from "lucide-react";
import { toPng } from "html-to-image";
import { useEffect, useMemo, useState } from "react";
import { api } from "./api";
import type {
  AttendanceStatus,
  AvailabilityStatus,
  Bootstrap,
  Lineup,
  PracticeDetail,
  PracticeSummary,
  User,
} from "./types";

type View = "myAvailability" | "myPractices" | "myAttendance" | "myTraining" | "teamWeek" | "teamTraining" | "teamRoster" | "teamRegattas" | "coachDashboard" | "coachBuilder" | "profile";

const viewGroups: Array<{ title: string; items: Array<{ id: View; label: string; icon: React.ReactNode; coachOnly?: boolean }> }> = [
  {
    title: "My",
    items: [
      { id: "myAvailability", label: "Availability", icon: <Users size={18} /> },
      { id: "myPractices", label: "Practices", icon: <ShipWheel size={18} /> },
      { id: "myAttendance", label: "Attendance", icon: <ClipboardList size={18} /> },
      { id: "myTraining", label: "Training", icon: <Dumbbell size={18} /> },
      { id: "profile", label: "Profile", icon: <Settings size={18} /> },
    ],
  },
  {
    title: "Team",
    items: [
      { id: "teamWeek", label: "Week View", icon: <CalendarDays size={18} /> },
      { id: "teamTraining", label: "Leaderboards", icon: <Activity size={18} /> },
      { id: "teamRoster", label: "Roster", icon: <Users size={18} /> },
      { id: "teamRegattas", label: "Regattas", icon: <Flag size={18} /> },
    ],
  },
  {
    title: "Coach",
    items: [
      { id: "coachDashboard", label: "Dashboard", icon: <UserPlus size={18} />, coachOnly: true },
      { id: "coachBuilder", label: "Builder", icon: <CalendarPlus size={18} />, coachOnly: true },
    ],
  },
];

type DraftSeat = {
  seat_number: number;
  role_label: string;
  user_id?: number;
};

type DraftLineup = {
  clientId: string;
  name: string;
  boat_type: string;
  shell: string;
  oars: string;
  seats: DraftSeat[];
};

type DragPayload = {
  type: string;
  id?: number;
  name?: string;
};

const shellOptions = ["Reveille", "12th Man", "Maroon", "Aggie Spirit", "Century", "Hudson 8"];

const availabilityLabels: Record<AvailabilityStatus, string> = {
  available: "Available",
  land_only: "Land only",
  maybe: "Maybe",
  out: "Out",
};

const attendanceLabels: Record<AttendanceStatus, string> = {
  present: "Present",
  excused: "Excused",
  absent: "Absent",
};

function App() {
  const [view, setView] = useState<View>("teamWeek");
  const [boot, setBoot] = useState<Bootstrap | null>(null);
  const [selectedPractice, setSelectedPractice] = useState<PracticeDetail | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState("");
  const [sidebarCollapsed, setSidebarCollapsed] = useState(false);

  async function load() {
    const data = await api.bootstrap();
    setBoot(data);
    const firstPractice = data.current_week.practices[0];
    setSelectedPractice(firstPractice ? await api.practice(firstPractice.id) : null);
  }

  useEffect(() => {
    let active = true;

    async function restore() {
      try {
        const data = await api.bootstrap();
        if (!active) return;
        setBoot(data);
        const firstPractice = data.current_week.practices[0];
        setSelectedPractice(firstPractice ? await api.practice(firstPractice.id) : null);
      } catch {
        localStorage.removeItem("crewgle_token");
      } finally {
        if (active) setLoading(false);
      }
    }

    void restore();
    return () => {
      active = false;
    };
  }, []);

  async function run(action: () => Promise<void>) {
    setError("");
    try {
      await action();
    } catch (err) {
      setError(err instanceof Error ? err.message : "Something went wrong");
    }
  }

  async function login(email: string, password: string) {
    const response = await api.login(email, password);
    localStorage.setItem("crewgle_token", response.token);
    await load();
  }

  async function logout() {
    await api.logout().catch(() => undefined);
    localStorage.removeItem("crewgle_token");
    setBoot(null);
    setSelectedPractice(null);
  }

  async function refreshPractice(id = selectedPractice?.id) {
    const data = await api.bootstrap();
    setBoot(data);
    setSelectedPractice(id ? await api.practice(id) : data.current_week.practices[0] ? await api.practice(data.current_week.practices[0].id) : null);
  }

  const activeView = viewGroups.flatMap((group) => group.items).find((item) => item.id === view);
  const canCoach = boot ? ["Admin", "Coach", "Officer"].includes(boot.user.role) : false;

  if (window.location.pathname === "/reset-password") {
    return <ResetPasswordScreen />;
  }

  if (loading) {
    return <div className="boot-screen">Crewgle</div>;
  }

  if (!boot) {
    return <AuthScreen onLogin={login} />;
  }

  return (
    <main className={`app-shell ${sidebarCollapsed ? "nav-collapsed" : ""}`}>
      <aside className="sidebar">
        <div>
          <div className="brand-row">
            <div className="brand-mark">Cg</div>
            <div className="brand-copy">
              <h1>Crewgle</h1>
              <p>Program control</p>
            </div>
            <button className="icon-button nav-toggle" type="button" onClick={() => setSidebarCollapsed((value) => !value)} aria-label={sidebarCollapsed ? "Expand navigation" : "Collapse navigation"}>
              <Menu size={18} />
            </button>
          </div>
          <nav className="rail">
            {viewGroups.map((group) => (
              <div className="nav-group" key={group.title}>
                <span>{group.title}</span>
                {group.items
                  .filter((item) => !item.coachOnly || canCoach)
                  .map((item) => (
                    <button key={item.id} className={view === item.id ? "active" : ""} type="button" onClick={() => setView(item.id)}>
                      {item.icon}
                      <span className="nav-label">{item.label}</span>
                    </button>
                  ))}
              </div>
            ))}
          </nav>
        </div>
        <div className="profile-strip">
          <div className="profile-copy">
            <strong>{boot.user.name}</strong>
            <span>{boot.user.role}</span>
          </div>
          <button className="icon-button" type="button" onClick={logout} aria-label="Log out">
            <LogOut size={18} />
          </button>
        </div>
      </aside>

      <section className="workspace">
        <header className="topbar">
          <div>
            <p className="eyebrow">Spring operations</p>
            <h2>{activeView?.label}</h2>
          </div>
          <button className="ghost-button" type="button" onClick={() => run(refreshPractice)}>
            <RefreshCw size={17} />
            Sync
          </button>
        </header>

        {error && <div className="error-banner">{error}</div>}

        {view === "teamWeek" && (
          <WeekView
            boot={boot}
            selectedPractice={selectedPractice}
            onSelect={(practice) => run(async () => setSelectedPractice(await api.practice(practice.id)))}
            onGenerate={(templateId, weekStart) => run(async () => {
              await api.generateWeek(templateId, weekStart);
              await refreshPractice();
            })}
          />
        )}
        {view === "myAvailability" && selectedPractice && (
          <AvailabilityView
            user={boot.user}
            practice={selectedPractice}
            onSet={(userId, status, notes) => run(async () => setSelectedPractice(await api.setAvailability({ practice_id: selectedPractice.id, user_id: userId, status, notes })))}
          />
        )}
        {view === "myPractices" && selectedPractice && (
          <MyPracticesView
            user={boot.user}
            boot={boot}
            onRequest={(payload) => run(async () => {
              const response = await api.requestSubstitution(payload);
              setSelectedPractice(response.practice);
            })}
          />
        )}
        {view === "coachDashboard" && (
          <CoachDashboard
            boot={boot}
            onInvite={(payload) => api.invite(payload)}
          />
        )}
        {view === "coachBuilder" && selectedPractice && (
          <CoachBuilderView
            boot={boot}
            selectedPractice={selectedPractice}
            onSelect={(practice) => run(async () => setSelectedPractice(await api.practice(practice.id)))}
            onOpenPractice={async (practice) => {
              const detail = await api.practice(practice.id);
              setSelectedPractice(detail);
              return detail;
            }}
            onCreatePractice={(payloads) => run(async () => {
              for (const payload of payloads) {
                await api.createPractice(payload);
              }
              await refreshPractice();
            })}
            onCreateLineup={async (payload) => {
              const detail = await api.createLineup(payload);
              setSelectedPractice(detail);
              return detail;
            }}
            onDeletePractice={(practiceId) => run(async () => {
              await api.deletePractice(practiceId);
              await refreshPractice();
            })}
          />
        )}
        {view === "myAttendance" && selectedPractice && (
          <AttendanceView
            key={selectedPractice.id}
            boot={boot}
            practice={selectedPractice}
            onRecord={(records) => run(async () => {
              setSelectedPractice(await api.recordAttendance(selectedPractice.id, records));
              await refreshPractice(selectedPractice.id);
            })}
            onAssignMakeup={(payload) => run(async () => {
              await api.assignMakeup(payload);
              await refreshPractice(selectedPractice.id);
            })}
            onUpdateMakeup={(id, payload) => run(async () => {
              await api.updateMakeup(id, payload);
              await refreshPractice(selectedPractice.id);
            })}
          />
        )}
        {view === "myTraining" && (
          <TrainingView
            boot={boot}
            onCreate={(payload) => run(async () => {
              await api.createWorkout(payload);
              await refreshPractice();
            })}
          />
        )}
        {view === "teamTraining" && <TeamTrainingView boot={boot} />}
        {view === "teamRoster" && <RosterView users={boot.users} />}
        {view === "teamRegattas" && <RegattaView boot={boot} />}
        {view === "profile" && (
          <ProfileView
            user={boot.user}
            onSave={(payload) => run(async () => {
              await api.updateProfile(payload);
              await refreshPractice();
            })}
          />
        )}
      </section>
    </main>
  );
}

function ResetPasswordScreen() {
  const token = new URLSearchParams(window.location.search).get("token") ?? "";
  const [password, setPassword] = useState("");
  const [confirm, setConfirm] = useState("");
  const [message, setMessage] = useState("");
  const [error, setError] = useState("");

  async function submit(event: React.FormEvent) {
    event.preventDefault();
    setError("");
    setMessage("");

    if (!token) {
      setError("Missing reset token.");
      return;
    }

    if (password.length < 6) {
      setError("Password must be at least 6 characters.");
      return;
    }

    if (password !== confirm) {
      setError("Passwords do not match.");
      return;
    }

    try {
      const response = await api.resetPassword(token, password);
      setMessage(response.message ?? "Password updated. You can now log in.");
    } catch (err) {
      setError(err instanceof Error ? err.message : "Could not reset password.");
    }
  }

  return (
    <main className="auth-shell">
      <section className="auth-panel">
        <div className="brand-row">
          <div className="brand-mark">Cg</div>
          <div>
            <h1>Reset password</h1>
            <p>Enter a new Crewgle password.</p>
          </div>
        </div>

        <form className="form-grid" onSubmit={submit}>
          <label>
            New password
            <input type="password" value={password} onChange={(event) => setPassword(event.target.value)} />
          </label>

          <label>
            Confirm password
            <input type="password" value={confirm} onChange={(event) => setConfirm(event.target.value)} />
          </label>

          <button className="primary-button" type="submit">
            <Check size={18} />
            Update password
          </button>

          {message && (
            <>
              <p className="form-success">{message}</p>

              <button
                type="button"
                className="secondary-button"
                onClick={() => {
                  window.location.href = "/";
                }}
              >
                Return to login
              </button>
            </>
          )}
          {error && <p className="form-error">{error}</p>}
        </form>
      </section>
    </main>
  );
}

function AuthScreen({ onLogin }: { onLogin: (email: string, password: string) => Promise<void> }) {
  const [mode, setMode] = useState<"login" | "register" | "forgot">("login");
  const [message, setMessage] = useState("");
  const [name, setName] = useState("");
  const [email, setEmail] = useState("coach@crewgle.test");
  const [password, setPassword] = useState("password");
  const [role, setRole] = useState("Athlete");
  const [error, setError] = useState("");

  async function submit(event: React.FormEvent) {
    event.preventDefault();
    setError("");
    setMessage("");

    try {
      if (mode === "forgot") {
        if (!email.trim()) {
          setError("Enter your email.");
          return;
        }
        const response = await api.forgotPassword(email.trim());
        setMessage(response.message ?? "If that email exists, a reset link has been sent.");
        return;
      }

      if (mode === "register") {
        await api.register({ name, email, password, role, squad_id: 1 });
      }
      await onLogin(email, password);
    } catch (err) {
      setError(err instanceof Error ? err.message : "Could not log in");
    }
  }

  return (
    <main className="auth-shell">
      <section className="auth-panel">
        <div className="brand-row">
          <div className="brand-mark">Cg</div>
          <div>
            <h1>Crewgle</h1>
            <p>Cleaner than the season spreadsheet.</p>
          </div>
        </div>
        <div className="mode-tabs">
          <button type="button" className={mode === "login" ? "active" : ""} onClick={() => setMode("login")}>Login</button>
          <button type="button" className={mode === "register" ? "active" : ""} onClick={() => setMode("register")}>Register</button>
        </div>
        <form className="form-grid" onSubmit={submit}>
          {mode === "register" && (
            <>
              <label>
                Name
                <input value={name} onChange={(event) => setName(event.target.value)} required />
              </label>
              <label>
                Email
                <input value={email} onChange={(event) => setEmail(event.target.value)} required />
              </label>
              <label>
                Role
                <select value={role} onChange={(event) => setRole(event.target.value)}>
                  <option>Athlete</option>
                  <option>Coxswain</option>
                  <option>Captain</option>
                </select>
              </label>
              <label>
                Password
                <input type="password" value={password} onChange={(event) => setPassword(event.target.value)} />
              </label>
              <button className="primary-button" type="submit">
                <Check size={18} />
                Register
              </button>
            </>
          )}
          {mode === "login" && (
            <>
              <label>
                Email
                <input value={email} onChange={(event) => setEmail(event.target.value)} />
              </label>
              <label>
                Password
                <input type="password" value={password} onChange={(event) => setPassword(event.target.value)} />
              </label>
              <button className="primary-button" type="submit">
                <Check size={18} />
                Login
              </button>
              <button 
                className="text-button"
                type="button"
                onClick={() => setMode("forgot")}
              >
                Forgot password?
              </button>
            </>
          )}
          {mode === "forgot" && (
            <>
              <label>
                Email
                <input type="email" value={email} onChange={(event) => setEmail(event.target.value)} />
              </label>
              <button 
                className="primary-button" 
                type="submit"
              >
                <Check size={18} />
                Submit
              </button>
              <button 
                className="text-button" 
                type="button" 
                onClick={() => setMode("login")}
              >
                Back to login
              </button>
            </>
           
          )}
          {message && <p className="form-success">{message}</p>}
          {error && <p className="form-error">{error}</p>}
        </form>
      </section>
    </main>
  );


}

function WeekView({
  boot,
  selectedPractice,
  onSelect,
  onGenerate,
}: {
  boot: Bootstrap;
  selectedPractice: PracticeDetail | null;
  onSelect: (practice: PracticeSummary) => void;
  onGenerate: (templateId: number, weekStart: string) => void;
}) {
  const [weekStart, setWeekStart] = useState(boot.current_week.week_start);
  const [templateId, setTemplateId] = useState(boot.templates[0]?.id ?? 1);

  async function exportBoard() {
    const node = document.getElementById("team-week-export-board");
    if (!node) return;
    const dataUrl = await toPng(node, { pixelRatio: 2, backgroundColor: "#f7f2df" });
    const link = document.createElement("a");
    link.download = `crewgle-lineups-${boot.lineup_board.week_start}.png`;
    link.href = dataUrl;
    link.click();
  }

  return (
    <div className="two-column">
      <section className="panel">
        <div className="export-actions compact">
          <PanelTitle icon={<CalendarDays size={18} />} title="Generated Week" />
          <button className="ghost-button" type="button" onClick={exportBoard}>
            <Download size={16} />
            PNG
          </button>
        </div>
        <div className="generate-row">
          <select value={templateId} onChange={(event) => setTemplateId(Number(event.target.value))}>
            {boot.templates.map((template) => (
              <option key={template.id} value={template.id}>
                {template.name}
              </option>
            ))}
          </select>
          <input type="date" value={weekStart} onChange={(event) => setWeekStart(event.target.value)} />
          <button className="primary-button" type="button" onClick={() => onGenerate(templateId, weekStart)}>
            <Plus size={16} />
            Generate
          </button>
        </div>
        <div className="practice-list">
          {boot.current_week.practices.map((practice) => (
            <button key={practice.id} className={`practice-card ${selectedPractice?.id === practice.id ? "active" : ""}`} type="button" onClick={() => onSelect(practice)}>
              <span>{formatDate(practice.practice_date)}</span>
              <strong>{practice.title}</strong>
              <small>{practice.start_time}-{practice.end_time || "open"} at {practice.location}</small>
              <div className="mini-metrics">
                <b>{practice.available_count} avail</b>
                <b>{practice.land_only_count} land</b>
                <b>{practice.out_count} out</b>
                <b>{practice.lineup_count} lineups</b>
              </div>
            </button>
          ))}
        </div>
      </section>

      <section className="panel hero-panel">
        {selectedPractice ? (
          <>
            <p className="eyebrow">{selectedPractice.squad_name} / {selectedPractice.practice_type}</p>
            <h2>{selectedPractice.title}</h2>
            <p>{selectedPractice.notes}</p>
            <div className="stat-grid">
              <Metric value={selectedPractice.available_count} label="available" />
              <Metric value={selectedPractice.present_count} label="present" />
              <Metric value={selectedPractice.lineup_count} label="lineups" />
            </div>
          </>
        ) : (
          <p>No practice selected.</p>
        )}
      </section>
      <div id="team-week-export-board" className="print-board screen-hidden">
        <h2>Crewgle Lineups / {boot.lineup_board.week_start}</h2>
        {boot.lineup_board.practices.map((practice) => (
          <article key={practice.id}>
            <h3>{formatDate(practice.practice_date)} / {practice.title}</h3>
            <div className="lineup-row">
              {practice.lineups.map((lineup) => <LineupCard key={lineup.id} lineup={lineup} />)}
            </div>
          </article>
        ))}
      </div>
    </div>
  );
}

function AvailabilityView({
  user,
  practice,
  onSet,
}: {
  user: User;
  practice: PracticeDetail;
  onSet: (userId: number, status: AvailabilityStatus, notes?: string) => void;
}) {
  const canManage = ["Admin", "Coach"].includes(user.role);
  const rows = canManage ? practice.athletes : practice.athletes.filter((athlete) => athlete.id === user.id);

  return (
    <section className="panel">
      <PanelTitle icon={<Users size={18} />} title={`Availability for ${practice.title}`} />
      <div className="roster-grid">
        {rows.map((athlete) => (
          <div className="roster-row" key={athlete.id}>
            <div>
              <strong>{athlete.name}</strong>
              <small>{athlete.role} / {athlete.side || "no side"} / water {athlete.water_count}, land {athlete.land_count}</small>
            </div>
            <Segmented
              value={athlete.availability_status}
              labels={availabilityLabels}
              values={["available", "land_only", "maybe", "out"]}
              onChange={(status) => onSet(athlete.id, status)}
            />
          </div>
        ))}
      </div>
    </section>
  );
}

function MyPracticesView({
  user,
  boot,
  onRequest,
}: {
  user: User;
  boot: Bootstrap;
  onRequest: (payload: Record<string, unknown>) => void;
}) {
  const assignments = boot.lineup_board.practices.flatMap((practice) =>
    practice.lineups.flatMap((lineup) =>
      lineup.entries
        .filter((entry) => entry.user_id === user.id)
        .map((entry) => ({ practice, lineup, entry })),
    ),
  );
  const availableSubs = boot.users.filter((candidate) => candidate.id !== user.id);

  return (
    <section className="panel">
      <PanelTitle icon={<ShipWheel size={18} />} title="My Practices" />
      <div className="lineup-row">
        {assignments.length === 0 ? (
          <p>No published lineup assignments for this week.</p>
        ) : (
          assignments.map(({ practice, lineup, entry }) => (
            <article className="makeup-card" key={`${lineup.id}-${entry.id}`}>
              <strong>{formatDate(practice.practice_date)} / {practice.title}</strong>
              <span>{lineup.name} / {entry.role_label === "Cox" ? "Cox" : `Seat ${entry.seat_number}`}</span>
              <p>{practice.location} / {practice.start_time}-{practice.end_time}</p>
              <div className="sub-row compact-sub">
                <select id={`sub-${lineup.id}-${entry.id}`}>
                  {availableSubs.map((candidate) => (
                    <option key={candidate.id} value={candidate.id}>{candidate.name}</option>
                  ))}
                </select>
                <button
                  className="ghost-button"
                  type="button"
                  onClick={() => {
                    const select = document.getElementById(`sub-${lineup.id}-${entry.id}`) as HTMLSelectElement | null;
                    onRequest({
                      lineup_id: lineup.id,
                      seat_number: entry.seat_number,
                      requested_sub_user_id: Number(select?.value),
                      requester_note: "Requested from My Practices",
                    });
                  }}
                >
                  Request Sub
                </button>
              </div>
            </article>
          ))
        )}
      </div>
    </section>
  );
}

function CoachDashboard({
  boot,
  onInvite,
}: {
  boot: Bootstrap;
  onInvite: (payload: Record<string, unknown>) => Promise<{ status: string; email: string; token: string; message: string }>;
}) {
  const [inviteEmail, setInviteEmail] = useState("");
  const [inviteRole, setInviteRole] = useState("Athlete");
  const [inviteSquad, setInviteSquad] = useState(boot.squads[0]?.id ?? 1);
  const [inviteResult, setInviteResult] = useState("");

  async function invite() {
    const response = await onInvite({ email: inviteEmail, role: inviteRole, squad_id: inviteSquad });
    setInviteResult(`Invite staged for ${response.email}`);
    setInviteEmail("");
  }

  return (
    <div className="coach-dashboard-grid">
      <section className="panel coach-hero">
        <PanelTitle icon={<UserPlus size={18} />} title="Coach Dashboard" />
        <h2>Program command center</h2>
        <p>Invites live here now, away from lineup construction. We can keep adding coach-only widgets here as the product gets more opinionated.</p>
      </section>

      <section className="panel invite-panel">
        <PanelTitle icon={<UserPlus size={18} />} title="Invite to Register" />
        <div className="form-grid">
          <label>
            Email
            <input value={inviteEmail} onChange={(event) => setInviteEmail(event.target.value)} placeholder="athlete@email.com" />
          </label>
          <div className="two-up">
            <label>
              Role
              <select value={inviteRole} onChange={(event) => setInviteRole(event.target.value)}>
                <option>Athlete</option>
                <option>Coxswain</option>
                <option>Captain</option>
                <option>Officer</option>
                <option>Coach</option>
              </select>
            </label>
            <label>
              Squad
              <select value={inviteSquad} onChange={(event) => setInviteSquad(Number(event.target.value))}>
                {boot.squads.map((squad) => <option key={squad.id} value={squad.id}>{squad.name}</option>)}
              </select>
            </label>
          </div>
          <button className="primary-button" type="button" onClick={() => void invite()} disabled={!inviteEmail}>
            <UserPlus size={17} />
            Stage Invite
          </button>
          {inviteResult && <p className="form-note">{inviteResult}</p>}
        </div>
      </section>

      <section className="panel">
        <PanelTitle icon={<Activity size={18} />} title="This Week" />
        <div className="stat-grid">
          <Metric value={boot.current_week.practices.length} label="practices" />
          <Metric value={boot.lineup_board.practices.reduce((total, practice) => total + practice.lineups.length, 0)} label="lineups" />
          <Metric value={boot.users.length} label="people" />
        </div>
      </section>
    </div>
  );
}

function CoachBuilderView({
  boot,
  selectedPractice,
  onSelect,
  onOpenPractice,
  onCreatePractice,
  onCreateLineup,
  onDeletePractice,
}: {
  boot: Bootstrap;
  selectedPractice: PracticeDetail;
  onSelect: (practice: PracticeSummary) => void;
  onOpenPractice: (practice: PracticeSummary) => Promise<PracticeDetail>;
  onCreatePractice: (payloads: Record<string, unknown>[]) => void;
  onCreateLineup: (payload: Record<string, unknown>) => Promise<PracticeDetail>;
  onDeletePractice: (practiceId: number) => void;
}) {
  const [title, setTitle] = useState("New Water Practice");
  const [practiceType, setPracticeType] = useState("water");
  const [startTime, setStartTime] = useState("06:00");
  const [endTime, setEndTime] = useState("08:00");
  const [location, setLocation] = useState("Lake Bryan");
  const [days, setDays] = useState<number[]>([1]);
  const [bulkOpen, setBulkOpen] = useState(false);
  const [modalPractice, setModalPractice] = useState<PracticeDetail | null>(null);
  const [selectedDay, setSelectedDay] = useState(selectedPractice.practice_date);

  const tallies = useMemo(() => {
    return boot.users.map((user) => {
      let water = 0;
      let land = 0;
      boot.lineup_board.practices.forEach((practice) => {
        const assigned = practice.lineups.some((lineup) => lineup.entries.some((entry) => entry.user_id === user.id));
        if (assigned && practice.practice_type === "water") water += 1;
        if (assigned && practice.practice_type !== "water") land += 1;
      });
      return { user, water, land };
    });
  }, [boot]);

  const selectedDayPractices = boot.lineup_board.practices.filter((practice) => practice.practice_date === selectedDay);

  function practicePayloads() {
    return days.map((weekday) => ({
      squad_id: 1,
      title,
      practice_type: practiceType,
      practice_date: dateForWeekday(boot.current_week.week_start, weekday),
      week_start: boot.current_week.week_start,
      start_time: startTime,
      end_time: endTime,
      location,
      notes: "Created in Coach Builder",
    }));
  }

  async function openPractice(practice: PracticeSummary) {
    setSelectedDay(practice.practice_date);
    const detail = await onOpenPractice(practice);
    setModalPractice(detail);
  }

  function createSinglePractice(weekday: number) {
    onCreatePractice([{
      squad_id: 1,
      title,
      practice_type: practiceType,
      practice_date: dateForWeekday(boot.current_week.week_start, weekday),
      week_start: boot.current_week.week_start,
      start_time: startTime,
      end_time: endTime,
      location,
      notes: "Created in Coach Builder",
    }]);
  }

  return (
    <div className="coach-builder-workspace">
      <section className="panel builder-calendar-panel">
        <div className="builder-toolbar">
          <PanelTitle icon={<CalendarPlus size={18} />} title="Coach Practice Builder" />
          <button className="primary-button" type="button" onClick={() => setBulkOpen(true)}>
            <Plus size={17} />
            Bulk Add Practices
          </button>
        </div>

        <div className="coach-week-board">
          {[0, 1, 2, 3, 4, 5, 6].map((weekday) => {
            const date = dateForWeekday(boot.current_week.week_start, weekday);
            const practices = boot.current_week.practices.filter((practice) => practice.practice_date === date);
            return (
              <article className={`calendar-day ${selectedDay === date ? "selected" : ""}`} key={weekday} onClick={() => setSelectedDay(date)}>
                <div className="calendar-day-head">
                  <strong>{weekdayName(weekday).slice(0, 3)}</strong>
                  <span>{formatShortDay(date)}</span>
                </div>
                <button className="day-add-button" type="button" onClick={(event) => {
                  event.stopPropagation();
                  createSinglePractice(weekday);
                }}>
                  <Plus size={15} />
                  Add
                </button>
                <div className="calendar-practice-stack">
                  {practices.map((practice) => (
                    <button
                      key={practice.id}
                      className={`calendar-practice ${selectedPractice.id === practice.id ? "active" : ""}`}
                      type="button"
                      onClick={(event) => {
                        event.stopPropagation();
                        setSelectedDay(practice.practice_date);
                        onSelect(practice);
                      }}
                      onDoubleClick={(event) => {
                        event.stopPropagation();
                        void openPractice(practice);
                      }}
                    >
                      <strong>{practice.title}</strong>
                      <span>{practice.start_time}-{practice.end_time || "open"}</span>
                      <small>{practice.location || practice.practice_type}</small>
                    </button>
                  ))}
                </div>
              </article>
            );
          })}
        </div>
      </section>

      <section className="panel builder-tally-panel">
        <PanelTitle icon={<Activity size={18} />} title="Weekly Tallies" />
        <div className="tally-list tallies-full">
          {tallies.map(({ user, water, land }) => (
            <div key={user.id}>
              <span>{user.name}</span>
              <b>W {water}</b>
              <b>L {land}</b>
            </div>
          ))}
        </div>
      </section>

      <section className="panel selected-day-panel">
        <PanelTitle icon={<ShipWheel size={18} />} title={`${formatDate(selectedDay)} Lineups`} />
        {selectedDayPractices.length === 0 ? (
          <p>No practices on this day yet.</p>
        ) : (
          <div className="selected-day-lineups">
            {selectedDayPractices.map((practice) => (
              <article key={practice.id} className="day-lineup-group">
                <button type="button" onClick={() => void openPractice(practice)}>
                  <strong>{practice.title}</strong>
                  <span>{practice.lineups.length} lineups</span>
                </button>
                <div className="lineup-row compact-lineups">
                  {practice.lineups.length === 0 ? <p>No lineups published.</p> : practice.lineups.map((lineup) => <LineupCard key={lineup.id} lineup={lineup} />)}
                </div>
              </article>
            ))}
          </div>
        )}
      </section>

      {bulkOpen && (
        <Modal title="Bulk Add Practices" onClose={() => setBulkOpen(false)}>
          <div className="form-grid">
            <label>Title<input value={title} onChange={(event) => setTitle(event.target.value)} /></label>
            <div className="two-up">
              <label>
                Type
                <select value={practiceType} onChange={(event) => setPracticeType(event.target.value)}>
                  <option value="water">Water</option>
                  <option value="erg">Erg</option>
                  <option value="lift">Lift</option>
                  <option value="meeting">Meeting</option>
                </select>
              </label>
              <label>Location<input value={location} onChange={(event) => setLocation(event.target.value)} /></label>
            </div>
            <div className="two-up">
              <label>Start<input type="time" value={startTime} onChange={(event) => setStartTime(event.target.value)} /></label>
              <label>End<input type="time" value={endTime} onChange={(event) => setEndTime(event.target.value)} /></label>
            </div>
            <div className="day-toggle-row">
              {[0, 1, 2, 3, 4, 5, 6].map((weekday) => (
                <label key={weekday} className="check-tile">
                  <input
                    type="checkbox"
                    checked={days.includes(weekday)}
                    onChange={() => setDays((current) => current.includes(weekday) ? current.filter((day) => day !== weekday) : [...current, weekday])}
                  />
                  {weekdayName(weekday).slice(0, 3)}
                </label>
              ))}
            </div>
            <button className="primary-button" type="button" disabled={days.length === 0} onClick={() => {
              onCreatePractice(practicePayloads());
              setBulkOpen(false);
            }}>
              <Plus size={17} />
              Create Practices
            </button>
          </div>
        </Modal>
      )}

      {modalPractice && (
        <PracticeLineupModal
          practice={modalPractice}
          onClose={() => setModalPractice(null)}
          onCreateLineup={async (payload) => {
            const detail = await onCreateLineup(payload);
            setModalPractice(detail);
            return detail;
          }}
          onDelete={() => {
            onDeletePractice(modalPractice.id);
            setModalPractice(null);
          }}
        />
      )}
    </div>
  );
}

function PracticeLineupModal({
  practice,
  onClose,
  onCreateLineup,
  onDelete,
}: {
  practice: PracticeDetail;
  onClose: () => void;
  onCreateLineup: (payload: Record<string, unknown>) => Promise<PracticeDetail>;
  onDelete: () => void;
}) {
  const [search, setSearch] = useState("");
  const [drafts, setDrafts] = useState<DraftLineup[]>([createDraftLineup("4+", 5)]);
  const [dragPayload, setDragPayload] = useState<DragPayload | null>(null);
  const available = practice.athletes.filter((athlete) => athlete.availability_status !== "out" && athlete.name.toLowerCase().includes(search.toLowerCase()));

  function beginDrag(event: React.DragEvent<HTMLElement>, payload: DragPayload) {
    setDragPayload(payload);
    event.dataTransfer.effectAllowed = "copy";
    event.dataTransfer.setData("application/json", JSON.stringify(payload));
  }

  function readDrop(event: React.DragEvent<HTMLElement>) {
    event.preventDefault();
    const raw = event.dataTransfer.getData("application/json");
    if (!raw) return null;
    return JSON.parse(raw) as DragPayload;
  }

  function assignSeatPayload(payload: DragPayload | null, clientId: string, seatNumber: number) {
    if (payload?.type !== "athlete" || !payload.id) return;
    setDrafts((current) => current.map((lineup) => lineup.clientId === clientId
      ? { ...lineup, seats: lineup.seats.map((seat) => seat.seat_number === seatNumber ? { ...seat, user_id: payload.id } : seat) }
      : lineup));
  }

  function assignSeat(event: React.DragEvent<HTMLElement>, clientId: string, seatNumber: number) {
    assignSeatPayload(readDrop(event), clientId, seatNumber);
    setDragPayload(null);
  }

  function assignShellPayload(payload: DragPayload | null, clientId: string) {
    if (payload?.type !== "shell" || !payload.name) return;
    const shell = payload.name;
    setDrafts((current) => current.map((lineup) => lineup.clientId === clientId ? { ...lineup, shell } : lineup));
  }

  function assignShell(event: React.DragEvent<HTMLElement>, clientId: string) {
    assignShellPayload(readDrop(event), clientId);
    setDragPayload(null);
  }

  function addDraft(boatType: string) {
    const seatCount = boatType === "8+" ? 9 : boatType === "2x" ? 2 : boatType === "1x" ? 1 : 5;
    setDrafts((current) => [...current, createDraftLineup(boatType, seatCount)]);
  }

  function athleteName(userId?: number) {
    return practice.athletes.find((athlete) => athlete.id === userId)?.name ?? "Drop athlete";
  }

  async function publishDrafts() {
    let latest = practice;
    for (const draft of drafts) {
      const entries = draft.seats
        .filter((seat) => seat.user_id)
        .map((seat) => ({ seat_number: seat.seat_number, role_label: seat.role_label, user_id: seat.user_id }));
      if (entries.length === 0) continue;
      latest = await onCreateLineup({
        practice_id: practice.id,
        name: draft.name,
        boat_type: draft.boat_type,
        shell: draft.shell,
        oars: draft.oars,
        entries,
      });
    }
    setDrafts([createDraftLineup("4+", 5)]);
    return latest;
  }

  return (
    <Modal title={`${practice.title} Lineup Builder`} onClose={onClose} wide>
      <div className="practice-modal-grid">
        <section className="lineup-drop-zone">
          <div className="modal-section-title">
            <h3>Lineups</h3>
            <div className="button-row">
              <button className="ghost-button" type="button" onClick={() => addDraft("4+")}>Add 4+</button>
              <button className="ghost-button" type="button" onClick={() => addDraft("8+")}>Add 8+</button>
            </div>
          </div>

          <div className="draft-lineup-grid">
            {drafts.map((draft) => (
              <article className="draft-boat" key={draft.clientId}>
                <div className="draft-boat-head">
                  <input value={draft.name} onChange={(event) => setDrafts((current) => current.map((lineup) => lineup.clientId === draft.clientId ? { ...lineup, name: event.target.value } : lineup))} />
                  <select value={draft.boat_type} onChange={(event) => {
                    const nextType = event.target.value;
                    const nextSeatCount = nextType === "8+" ? 9 : nextType === "2x" ? 2 : nextType === "1x" ? 1 : 5;
                    setDrafts((current) => current.map((lineup) => lineup.clientId === draft.clientId ? { ...createDraftLineup(nextType, nextSeatCount), clientId: lineup.clientId, name: lineup.name } : lineup));
                  }}>
                    <option>4+</option>
                    <option>8+</option>
                    <option>2x</option>
                    <option>1x</option>
                  </select>
                </div>
                <div
                  className={`shell-drop ${draft.shell ? "filled" : ""}`}
                  onDragOver={(event) => event.preventDefault()}
                  onDrop={(event) => assignShell(event, draft.clientId)}
                  onMouseUp={() => {
                    assignShellPayload(dragPayload, draft.clientId);
                    setDragPayload(null);
                  }}
                >
                  {draft.shell || "Drop shell here"}
                </div>
                <div className="boat-seats">
                  {draft.seats.map((seat) => (
                    <button
                      key={seat.seat_number}
                      className={`seat-drop ${seat.user_id ? "filled" : ""}`}
                      type="button"
                      onDragOver={(event) => event.preventDefault()}
                      onDrop={(event) => assignSeat(event, draft.clientId, seat.seat_number)}
                      onMouseUp={() => {
                        assignSeatPayload(dragPayload, draft.clientId, seat.seat_number);
                        setDragPayload(null);
                      }}
                    >
                      <span>{seat.role_label === "Cox" ? "Cox" : `Seat ${seat.seat_number}`}</span>
                      <strong>{athleteName(seat.user_id)}</strong>
                    </button>
                  ))}
                </div>
              </article>
            ))}
          </div>

          <div className="published-strip">
            <h3>Published</h3>
            <div className="lineup-row compact-lineups">
              {practice.lineups.length === 0 ? <p>No published lineups yet.</p> : practice.lineups.map((lineup) => <LineupCard key={lineup.id} lineup={lineup} />)}
            </div>
          </div>
        </section>

        <aside className="resource-column">
          <section className="resource-panel people-panel">
            <h3>Available People</h3>
            <input value={search} onChange={(event) => setSearch(event.target.value)} placeholder="Filter search" />
            <div className="draggable-list">
              {available.map((athlete) => (
                <div
                  key={athlete.id}
                  className="drag-pill athlete-pill"
                  draggable
                  onDragStart={(event) => beginDrag(event, { type: "athlete", id: athlete.id })}
                  onDragEnd={() => setDragPayload(null)}
                  onMouseDown={() => setDragPayload({ type: "athlete", id: athlete.id })}
                >
                  <strong>{athlete.name}</strong>
                  <span>{athlete.side || athlete.role} / {availabilityLabels[athlete.availability_status]}</span>
                </div>
              ))}
            </div>
          </section>

          <section className="resource-panel shells-panel">
            <h3>Available Shells</h3>
            <div className="shell-list">
              {shellOptions.map((shell) => (
                <div
                  key={shell}
                  className="drag-pill shell-pill"
                  draggable
                  onDragStart={(event) => beginDrag(event, { type: "shell", name: shell })}
                  onDragEnd={() => setDragPayload(null)}
                  onMouseDown={() => setDragPayload({ type: "shell", name: shell })}
                >
                  {shell}
                </div>
              ))}
            </div>
          </section>
        </aside>
      </div>

      <div className="modal-actions split-actions">
        <button className="danger-button" type="button" onClick={onDelete}>
          <Trash2 size={17} />
          Delete Practice
        </button>
        <div className="button-row">
          <button className="ghost-button" type="button" onClick={onClose}>Close</button>
          <button className="primary-button" type="button" onClick={() => void publishDrafts()}>
            <Plus size={17} />
            Publish Draft Lineups
          </button>
        </div>
      </div>
    </Modal>
  );
}

function Modal({ title, children, onClose, wide = false }: { title: string; children: React.ReactNode; onClose: () => void; wide?: boolean }) {
  return (
    <div className="modal-backdrop" role="presentation">
      <section className={`modal-window ${wide ? "wide-modal" : ""}`} role="dialog" aria-modal="true" aria-label={title}>
        <div className="modal-header">
          <h2>{title}</h2>
          <button className="icon-button" type="button" onClick={onClose} aria-label="Close modal">
            <X size={18} />
          </button>
        </div>
        {children}
      </section>
    </div>
  );
}

function createDraftLineup(boatType: string, seatCount: number): DraftLineup {
  const id = typeof crypto !== "undefined" && "randomUUID" in crypto ? crypto.randomUUID() : `${Date.now()}-${Math.random()}`;
  return {
    clientId: id,
    name: `New ${boatType}`,
    boat_type: boatType,
    shell: "",
    oars: "",
    seats: Array.from({ length: seatCount }, (_, index) => ({
      seat_number: index + 1,
      role_label: boatType.endsWith("+") && index + 1 === seatCount ? "Cox" : "Seat",
    })),
  };
}

function AttendanceView({
  boot,
  practice,
  onRecord,
  onAssignMakeup,
  onUpdateMakeup,
}: {
  boot: Bootstrap;
  practice: PracticeDetail;
  onRecord: (records: Array<{ user_id: number; status: AttendanceStatus }>) => void;
  onAssignMakeup: (payload: Record<string, unknown>) => void;
  onUpdateMakeup: (id: number, payload: Record<string, unknown>) => void;
}) {
  const initial = useMemo(() => {
    const map: Record<number, AttendanceStatus> = {};
    practice.athletes.forEach((athlete) => {
      const existing = practice.attendance.find((entry) => entry.user_id === athlete.id);
      map[athlete.id] = existing?.status ?? "absent";
    });
    return map;
  }, [practice]);
  const [records, setRecords] = useState(initial);
  const [makeupUser, setMakeupUser] = useState(practice.athletes[0]?.id ?? 0);

  return (
    <div className="two-column wide">
      <section className="panel">
        <PanelTitle icon={<ClipboardList size={18} />} title={`Attendance / ${practice.title}`} />
        <div className="roster-grid">
          {practice.athletes.map((athlete) => (
            <div className="roster-row" key={athlete.id}>
              <div>
                <strong>{athlete.name}</strong>
                <small>{athlete.role} / availability {availabilityLabels[athlete.availability_status]}</small>
              </div>
              <Segmented
                value={records[athlete.id]}
                labels={attendanceLabels}
                values={["present", "excused", "absent"]}
                onChange={(status) => setRecords((current) => ({ ...current, [athlete.id]: status }))}
              />
            </div>
          ))}
        </div>
        <button className="primary-button" type="button" onClick={() => onRecord(Object.entries(records).map(([userId, status]) => ({ user_id: Number(userId), status })))}>
          <Check size={17} />
          Save Attendance
        </button>
      </section>

      <section className="panel">
        <PanelTitle icon={<Activity size={18} />} title="Misses & Makeups" />
        <div className="makeup-form">
          <select value={makeupUser} onChange={(event) => setMakeupUser(Number(event.target.value))}>
            {boot.users.map((user) => <option key={user.id} value={user.id}>{user.name}</option>)}
          </select>
          <button className="ghost-button" type="button" onClick={() => onAssignMakeup({ user_id: makeupUser, practice_id: practice.id, title: "Missed practice makeup", description: "45 minutes UT2 or coach-approved equivalent.", due_date: "2026-06-15" })}>
            Assign Makeup
          </button>
        </div>
        <div className="stack">
          {boot.attendance.makeups.map((makeup) => (
            <article className="makeup-card" key={makeup.id}>
              <strong>{makeup.user_name}: {makeup.title}</strong>
              <span>{makeup.status} / due {makeup.due_date || "TBD"}</span>
              <p>{makeup.description}</p>
              {makeup.status !== "accepted" && (
                <button className="ghost-button" type="button" onClick={() => onUpdateMakeup(makeup.id, { status: "accepted", completion_notes: "Accepted by coach." })}>
                  Accept
                </button>
              )}
            </article>
          ))}
        </div>
      </section>
    </div>
  );
}

function TrainingView({ boot, onCreate }: { boot: Bootstrap; onCreate: (payload: Record<string, unknown>) => void }) {
  const [name, setName] = useState("Steady state");
  const [workoutDate, setWorkoutDate] = useState("2026-06-11");
  const [type, setType] = useState("UT2");
  const [meters, setMeters] = useState(12000);
  const [time, setTime] = useState("53:00");
  const [testPiece, setTestPiece] = useState("");

  return (
    <div className="two-column wide">
      <section className="panel">
        <PanelTitle icon={<Dumbbell size={18} />} title="Submit Erg Workout" />
        <form className="form-grid" onSubmit={(event) => {
          event.preventDefault();
          onCreate({ workout_date: workoutDate, name, workout_type: type, meters, total_seconds: parseDuration(time), is_test: Boolean(testPiece), test_piece: testPiece });
        }}>
          <input value={name} onChange={(event) => setName(event.target.value)} />
          <div className="three-up">
            <input type="date" value={workoutDate} onChange={(event) => setWorkoutDate(event.target.value)} />
            <select value={type} onChange={(event) => setType(event.target.value)}>
              {["UT3", "UT2", "UT1", "AT", "TR", "AC", "VO2", "AP"].map((item) => <option key={item}>{item}</option>)}
            </select>
            <input value={testPiece} onChange={(event) => setTestPiece(event.target.value)} placeholder="Test piece" />
          </div>
          <div className="two-up">
            <input type="number" value={meters} onChange={(event) => setMeters(Number(event.target.value))} />
            <input value={time} onChange={(event) => setTime(event.target.value)} placeholder="mm:ss" />
          </div>
          <button className="primary-button" type="submit">
            <Plus size={17} />
            Submit
          </button>
        </form>
      </section>
      <section className="panel">
        <PanelTitle icon={<Activity size={18} />} title="Progress" />
        <div className="stat-grid">
          <Metric value={boot.workouts.totals.workout_count} label="workouts" />
          <Metric value={Math.round(boot.workouts.totals.meters / 1000)} label="km" />
          <Metric value={formatSplit(boot.workouts.totals.avg_split_seconds)} label="avg split" />
        </div>
        <div className="bar-list">
          {boot.workouts.by_type.map((row) => (
            <div key={row.workout_type}>
              <span>{row.workout_type}</span>
              <div><i style={{ width: `${Math.min(100, Number(row.meters) / 200)}%` }} /></div>
              <b>{row.meters}m</b>
            </div>
          ))}
        </div>
        <div className="stack">
          {boot.workouts.test_progress.map((row) => (
            <p key={`${row.test_piece}-${row.workout_date}`}>{row.test_piece} / {row.workout_date} / {formatSplit(row.split_seconds)}</p>
          ))}
        </div>
      </section>
    </div>
  );
}

function TeamTrainingView({ boot }: { boot: Bootstrap }) {
  return (
    <div className="two-column wide">
      <section className="panel">
        <PanelTitle icon={<Activity size={18} />} title="Benchmark Leaderboards" />
        <div className="data-table leaderboard-table">
          <div className="table-head"><span>Name</span><span>Squad</span><span>Piece</span><span>Split</span></div>
          {boot.workouts.leaderboard.map((row) => (
            <div key={`${row.name}-${row.test_piece}`}>
              <strong>{row.name}</strong>
              <span>{row.squads}</span>
              <span>{row.test_piece}</span>
              <span>{formatSplit(row.best_split_seconds)}</span>
            </div>
          ))}
        </div>
      </section>
      <section className="panel">
        <PanelTitle icon={<Dumbbell size={18} />} title="Team Volume" />
        <div className="bar-list">
          {boot.workouts.by_type.map((row) => (
            <div key={row.workout_type}>
              <span>{row.workout_type}</span>
              <div><i style={{ width: `${Math.min(100, Number(row.meters) / 200)}%` }} /></div>
              <b>{row.meters}m</b>
            </div>
          ))}
        </div>
      </section>
    </div>
  );
}

function RosterView({ users }: { users: User[] }) {
  return (
    <section className="panel">
      <PanelTitle icon={<Users size={18} />} title="Team Roster" />
      <div className="data-table">
        <div className="table-head">
          <span>Name</span><span>Role</span><span>Squad</span><span>Side</span><span>Contact</span>
        </div>
        {users.map((user) => (
          <div key={user.id}>
            <strong>{user.name}</strong>
            <span>{user.role}</span>
            <span>{String(user.squads ?? "")}</span>
            <span className="side-cell"><i className={`side-dot ${sideClass(user.side)}`} />{sideShort(user.side)}</span>
            <span className="contact-icons">
              <a href={`mailto:${user.email}`} aria-label={`Email ${user.name}`}><Mail size={16} /></a>
              <a href={`tel:${user.phone}`} aria-label={`Call ${user.name}`}><Phone size={16} /></a>
              {user.instagram_url && <a href={user.instagram_url} aria-label={`${user.name} Instagram`}><LinkIcon size={16} /></a>}
            </span>
          </div>
        ))}
      </div>
    </section>
  );
}

function ProfileView({ user, onSave }: { user: User; onSave: (payload: Record<string, unknown>) => void }) {
  const [phone, setPhone] = useState(user.phone ?? "");
  const [side, setSide] = useState(user.side ?? "");
  const [classYear, setClassYear] = useState(user.class_year ?? "");
  const [instagramUrl, setInstagramUrl] = useState(user.instagram_url ?? "");

  return (
    <section className="panel profile-panel">
      <PanelTitle icon={<Settings size={18} />} title="Profile Settings" />
      <div className="form-grid">
        <label>Phone<input value={phone} onChange={(event) => setPhone(event.target.value)} /></label>
        <div className="two-up">
          <label>
            Side
            <select value={side} onChange={(event) => setSide(event.target.value)}>
              <option value="">Either / NA</option>
              <option>Port</option>
              <option>Starboard</option>
            </select>
          </label>
          <label>Class year<input value={classYear} onChange={(event) => setClassYear(event.target.value)} /></label>
        </div>
        <label>Instagram URL<input value={instagramUrl} onChange={(event) => setInstagramUrl(event.target.value)} /></label>
        <button className="primary-button" type="button" onClick={() => onSave({ phone, side, class_year: classYear, instagram_url: instagramUrl })}>
          <Check size={17} />
          Save Profile
        </button>
      </div>
    </section>
  );
}

function RegattaView({ boot }: { boot: Bootstrap }) {
  return (
    <section className="panel">
      <PanelTitle icon={<Flag size={18} />} title="Regatta Ops" />
      <div className="regatta-grid">
        {boot.current_week.regattas.map((regatta) => (
          <article key={regatta.id} className="makeup-card">
            <strong>{regatta.name}</strong>
            <span>{regatta.start_date} / {regatta.location}</span>
            <p>{regatta.notes}</p>
          </article>
        ))}
      </div>
    </section>
  );
}

function LineupCard({ lineup }: { lineup: Lineup }) {
  return (
    <article className="lineup-card">
      <div>
        <strong>{lineup.name}</strong>
        <small>{lineup.boat_type} / {lineup.shell} / oars {lineup.oars}</small>
      </div>
      <ol>
        {lineup.entries.map((entry) => (
          <li key={entry.id}>
            <span>{entry.role_label === "Cox" ? "C" : entry.seat_number}</span>
            <b>{entry.user_name}</b>
            <em>{entry.availability_status}</em>
          </li>
        ))}
      </ol>
    </article>
  );
}

function PanelTitle({ icon, title }: { icon: React.ReactNode; title: string }) {
  return (
    <div className="panel-title">
      {icon}
      <h3>{title}</h3>
    </div>
  );
}

function Metric({ value, label }: { value: number | string; label: string }) {
  return (
    <div className="metric">
      <strong>{value}</strong>
      <span>{label}</span>
    </div>
  );
}

function Segmented<T extends string>({
  value,
  values,
  labels,
  onChange,
}: {
  value: T;
  values: T[];
  labels: Record<T, string>;
  onChange: (value: T) => void;
}) {
  return (
    <div className="segmented">
      {values.map((item) => (
        <button key={item} type="button" className={value === item ? "active" : ""} title={labels[item]} onClick={() => onChange(item)}>
          {labels[item].slice(0, 1)}
        </button>
      ))}
    </div>
  );
}

function formatDate(value: string) {
  return new Intl.DateTimeFormat(undefined, { weekday: "short", month: "short", day: "numeric" }).format(new Date(`${value}T00:00:00`));
}

function formatShortDay(value: string) {
  return new Intl.DateTimeFormat(undefined, { month: "numeric", day: "numeric" }).format(new Date(`${value}T00:00:00`));
}

function weekdayName(value: number) {
  return ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"][value] ?? "Day";
}

function dateForWeekday(weekStart: string, weekday: number) {
  const start = new Date(`${weekStart}T00:00:00`);
  const offset = weekday - start.getDay();
  start.setDate(start.getDate() + offset);
  return start.toISOString().slice(0, 10);
}

function sideClass(side: string) {
  const value = side.toLowerCase();
  if (value.includes("port")) return "port";
  if (value.includes("star")) return "starboard";
  return "either";
}

function sideShort(side: string) {
  const value = side.toLowerCase();
  if (value.includes("port")) return "P";
  if (value.includes("star")) return "S";
  return "E";
}

function parseDuration(value: string) {
  const parts = value.split(":").map(Number);
  if (parts.length === 3) return parts[0] * 3600 + parts[1] * 60 + parts[2];
  if (parts.length === 2) return parts[0] * 60 + parts[1];
  return Number(value);
}

function formatSplit(seconds: number) {
  if (!seconds) return "0:00";
  const mins = Math.floor(seconds / 60);
  const secs = Math.round(seconds % 60).toString().padStart(2, "0");
  return `${mins}:${secs}`;
}

export default App;
