// Types for MCC frontend based on database schema

// Enums matching database schema
export type CommandStatus =
  | 'pending'
  | 'scheduled'
  | 'ongoing'
  | 'cancelled'
  | 'failed'
  | 'completed';

export type SessionStatus =
  | 'pending'
  | 'scheduled'
  | 'ongoing'
  | 'completed';

export type MainPacketType = 'uplink' | 'downlink';

// Database table interfaces
export interface Session {
  id: number;
  start_time: string; // ISO datetime string
  end_time: string | null;
  status: SessionStatus;
}

export interface Packet {
  id: number;
  session_id: number;
  raw_data: string;
  type: MainPacketType;
  subtype: string; // enum - TODO: Define specific values when known
  payload_data: string;
  created_on: string;
  offset: number;
}

export interface Command {
  name: string;
  id: number;
  params: string;
  format: string;
  data_size: number;
  total_size: number;
}

export interface SentCommand extends Command {
  type: number; // References master.commands.id
  status: CommandStatus;
}

export interface Telemetry {
  id: number;
  type: number; // References master.telemetry.id
  value: string;
}

// Master table interfaces
export interface MasterCommand {
  id: number;
  name: string;
  params: string;
  format: string;
  data_size: number;
  total_size: number;
}

export interface MasterTelemetry {
  id: number;
  name: string;
  format: string;
  data_size: number;
  total_size: number;
}

// Extended interfaces with joined data (for frontend display)
export interface TelemetryWithMaster extends Telemetry {
  telemetry_name?: string;
  telemetry_format?: string;
  created_on?: string;
}

export interface CommandWithMaster extends Command {
  command_name?: string;
  command_format?: string;
  created_on?: string;
}

// User-related interfaces
export interface UserData {
  id: number;
  call_sign: string;
  first_name: string;
  last_name: string | null;
  phone_number: string | null;
}

export interface UserLogin {
  id: number;
  email: string;
  created_on: string;
  user_data_id: number;
  email_verification_token: string | null;
}
