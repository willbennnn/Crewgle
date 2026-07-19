import type {
  AttendanceStatus,
  AvailabilityStatus,
  Bootstrap,
  PracticeDetail,
  User,
  WeekOverview,
} from "./types";

const API_BASE = import.meta.env.VITE_API_BASE ?? "http://localhost:8080";

type LoginResponse = {
  token: string;
  user: User;
};

async function request<T>(path: string, options: RequestInit = {}): Promise<T> {
  const headers = new Headers(options.headers);
  const token = localStorage.getItem("crewgle_token");

  if (options.body && !headers.has("Content-Type")) {
    headers.set("Content-Type", "application/json");
  }

  if (token) {
    headers.set("Authorization", `Bearer ${token}`);
  }

  const response = await fetch(`${API_BASE}${path}`, { ...options, headers });
  const payload = await response.json().catch(() => null);

  if (!response.ok) {
    throw new Error(payload?.error ?? "Crewgle request failed");
  }

  return payload as T;
}

export const api = {
  login(email: string, password: string) {
    return request<LoginResponse>("/api/auth/login", {
      method: "POST",
      body: JSON.stringify({ email, password }),
    });
  },

  register(input: Record<string, unknown>) {
    return request<{ user: User }>("/api/auth/register", {
      method: "POST",
      body: JSON.stringify(input),
    });
  },

  updateProfile(input: Record<string, unknown>) {
    return request<{ user: User }>("/api/profile", {
      method: "PATCH",
      body: JSON.stringify(input),
    });
  },

  invite(input: Record<string, unknown>) {
    return request<{ status: string; email: string; token: string; message: string }>("/api/invitations", {
      method: "POST",
      body: JSON.stringify(input),
    });
  },

  me() {
    return request<{ user: User }>("/api/auth/me");
  },

  forgotPassword(email: string) {
    return request<{ status: string; message?: string;}>("/api/auth/reset-password", {
      method: "POST",
      body: JSON.stringify({ email }),
    });
  },

  resetPassword(token: string, password: string) {
    return request<{ message: string }>("/api/auth/reset-password/confirm", {
      method: "POST",
      body: JSON.stringify({ token, password })
    });
  },

  logout() {
    return request<{ status: string }>("/api/auth/logout", { method: "POST" });
  },

  bootstrap() {
    return request<Bootstrap>("/api/bootstrap");
  },

  week(weekStart: string) {
    return request<WeekOverview>(`/api/weeks/${weekStart}`);
  },

  practice(id: number) {
    return request<PracticeDetail>(`/api/practices/${id}`);
  },

  createPractice(input: Record<string, unknown>) {
    return request<{ week: WeekOverview; practice: PracticeDetail }>("/api/practices", {
      method: "POST",
      body: JSON.stringify(input),
    });
  },

  deletePractice(id: number) {
    return request<{ status: string; week: WeekOverview }>(`/api/practices/${id}`, {
      method: "DELETE",
    });
  },

  generateWeek(templateId: number, weekStart: string) {
    return request<WeekOverview>(`/api/practice-templates/${templateId}/generate-week`, {
      method: "POST",
      body: JSON.stringify({ week_start: weekStart }),
    });
  },

  setAvailability(input: {
    practice_id: number;
    user_id?: number;
    status: AvailabilityStatus;
    notes?: string;
  }) {
    return request<PracticeDetail>("/api/availability", {
      method: "POST",
      body: JSON.stringify(input),
    });
  },

  createLineup(input: Record<string, unknown>) {
    return request<PracticeDetail>("/api/lineups", {
      method: "POST",
      body: JSON.stringify(input),
    });
  },

  substitute(lineupId: number, input: Record<string, unknown>) {
    return request<PracticeDetail>(`/api/lineups/${lineupId}/substitutions`, {
      method: "POST",
      body: JSON.stringify(input),
    });
  },

  requestSubstitution(input: Record<string, unknown>) {
    return request<{ status: string; message: string; practice: PracticeDetail }>("/api/substitution-requests", {
      method: "POST",
      body: JSON.stringify(input),
    });
  },

  recordAttendance(practiceId: number, records: Array<{ user_id: number; status: AttendanceStatus; notes?: string }>) {
    return request<PracticeDetail>("/api/attendance", {
      method: "POST",
      body: JSON.stringify({ practice_id: practiceId, records }),
    });
  },

  assignMakeup(input: Record<string, unknown>) {
    return request<Bootstrap["attendance"]>("/api/makeups", {
      method: "POST",
      body: JSON.stringify(input),
    });
  },

  updateMakeup(id: number, input: Record<string, unknown>) {
    return request<Bootstrap["attendance"]>(`/api/makeups/${id}`, {
      method: "PATCH",
      body: JSON.stringify(input),
    });
  },

  createWorkout(input: Record<string, unknown>) {
    return request<Bootstrap["workouts"]>("/api/workouts", {
      method: "POST",
      body: JSON.stringify(input),
    });
  }
};
