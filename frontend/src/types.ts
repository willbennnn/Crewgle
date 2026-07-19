export type Role = "Admin" | "Coach" | "Officer" | "Captain" | "Coxswain" | "Athlete";
export type AvailabilityStatus = "available" | "land_only" | "maybe" | "out";
export type AttendanceStatus = "present" | "absent" | "excused";

export type User = {
  id: number;
  name: string;
  email: string;
  role: Role;
  side: string;
  class_year: string;
  phone: string;
  instagram_url: string;
  status: string;
  squads?: Squad[] | string;
};

export type Squad = {
  id: number;
  name: string;
  level: string;
};

export type TemplateSlot = {
  id: number;
  weekday: number;
  title: string;
  practice_type: PracticeType;
  start_time: string;
  end_time: string;
  location: string;
  notes: string;
};

export type PracticeTemplate = {
  id: number;
  name: string;
  squad_id: number;
  squad_name: string;
  created_by_user_id: number;
  created_by_name: string;
  active: number;
  created_at: string;
  slots: TemplateSlot[];
};

export type PracticeType = "water" | "erg" | "lift" | "meeting";

export type PracticeSummary = {
  id: number;
  template_id: number;
  squad_id: number;
  squad_name: string;
  title: string;
  practice_type: PracticeType;
  practice_date: string;
  week_start: string;
  start_time: string;
  end_time: string;
  location: string;
  notes: string;
  status: string;
  available_count: number;
  land_only_count: number;
  maybe_count: number;
  out_count: number;
  present_count: number;
  lineup_count: number;
};

export type AthleteContext = {
  id: number;
  name: string;
  role: Role;
  side: string;
  class_year: string;
  availability_status: AvailabilityStatus;
  availability_notes: string;
  water_count: number;
  land_count: number;
};

export type AttendanceRecord = {
  id: number;
  user_id: number;
  user_name: string;
  role: Role;
  practice_id: number;
  status: AttendanceStatus;
  notes: string;
};

export type LineupEntry = {
  id: number;
  seat_number: number;
  role_label: string;
  user_id: number;
  user_name: string;
  role: Role;
  side: string;
  availability_status: AvailabilityStatus;
};

export type Lineup = {
  id: number;
  practice_id: number;
  name: string;
  boat_type: string;
  shell: string;
  oars: string;
  status: string;
  created_by_user_id: number;
  notes: string;
  entries: LineupEntry[];
  substitutions: Array<{
    id: number;
    seat_number: number;
    original_user_name: string;
    new_user_name: string;
    reason: string;
    created_by_name: string;
    created_at: string;
  }>;
};

export type PracticeDetail = PracticeSummary & {
  athletes: AthleteContext[];
  attendance: AttendanceRecord[];
  lineups: Lineup[];
};

export type WeekOverview = {
  week_start: string;
  practices: PracticeSummary[];
  makeups_due: MakeupWorkout[];
  regattas: Regatta[];
};

export type MakeupWorkout = {
  id: number;
  user_id: number;
  user_name: string;
  practice_id: number | null;
  practice_title: string | null;
  practice_date: string | null;
  title: string;
  description: string;
  due_date: string;
  status: string;
  completion_notes: string;
};

export type AttendanceReport = {
  misses: AttendanceRecord[];
  makeups: MakeupWorkout[];
};

export type WorkoutSummary = {
  user_id: number;
  totals: {
    workout_count: number;
    meters: number;
    avg_split_seconds: number;
  };
  by_type: Array<{ workout_type: string; count: number; meters: number }>;
  test_progress: Array<{
    test_piece: string;
    workout_date: string;
    meters: number;
    total_seconds: number;
    split_seconds: number;
  }>;
  recent: Array<{
    id: number;
    workout_date: string;
    name: string;
    workout_type: string;
    meters: number;
    total_seconds: number;
    split_seconds: number;
    is_test: number;
    test_piece: string;
    photo_url: string;
    notes: string;
  }>;
  leaderboard: Array<{
    name: string;
    squads: string;
    test_piece: string;
    best_split_seconds: number;
  }>;
};

export type Regatta = {
  id: number;
  name: string;
  start_date: string;
  end_date: string;
  location: string;
  notes: string;
};

export type LineupBoard = {
  week_start: string;
  practices: Array<PracticeSummary & { lineups: Lineup[] }>;
};

export type Bootstrap = {
  user: User;
  users: User[];
  squads: Squad[];
  templates: PracticeTemplate[];
  current_week: WeekOverview;
  lineup_board: LineupBoard;
  attendance: AttendanceReport;
  workouts: WorkoutSummary;
};
