// Shared types based on database schema for ARO and MCC frontends

// Enums matching database schema
export type AROCommandStatus =
  | 'pending'
  | 'scheduled'
  | 'taken'
  | 'cancelled'
  | 'failed'
  | 'completed';

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
export interface ARORequest {
  id: number;
  aro_id: number;
  latitude: number; // decimal in DB, number in TS
  longitude: number; // decimal in DB, number in TS
  status: AROCommandStatus;
  created_on: string; // ISO datetime string
  request_sent_to_obc_on: string | null;
  pic_taken_on: string | null;
  pic_transmitted_on: string | null;
  packet_id: number | null;
}

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
  id: number;
  status: CommandStatus;
  type: number; // References master.commands.id
  params: string;
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