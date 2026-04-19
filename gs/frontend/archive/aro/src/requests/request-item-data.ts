
export type AROCommandStatus =
  | 'pending'
  | 'scheduled'
  | 'taken'
  | 'cancelled'
  | 'failed'
  | 'completed';

export interface RequestItemData {
  id: number;
  aro_id: number;
  latitude: number;
  longitude: number;
  status: AROCommandStatus;
  created_on: Date;
  request_sent_to_obc_on: Date | null;
  pic_taken_on: Date | null;
  pic_transmitted_on: Date | null;
  packet_id?: number | null;
  cancellable_after: Date;
}