import type { Command } from "./types";

// Extended command interface with parameter details
export interface CommandParameter {
  name: string;
  type: 'int' | 'float' | 'string' | 'boolean'; // TODO: this needs to be confirmed with actual types used
  size?: number; // size in bytes for int/float
}

export interface ExtendedCommand extends Command {
  parameters: CommandParameter[];
}

const mockCommandsList: ExtendedCommand[] = [
  {
    id: 1,
    name: "RTC Sync",
    params: "time",
    format: "int 7 bytes",
    data_size: 7,
    total_size: 8,
    // data above is the standard format for each command
    // below is the parameter breakdown, which will need to be created for the actual commands as well
    parameters: [
      {
        name: "time",
        type: "int",
        size: 7,
      }
    ]
  },
  {
    id: 2,
    name: "Manually activate an emergency mode for a specified amount of time",
    params: "mode_state_number,time",
    format: "int 1 byte, int 7 bytes",
    data_size: 8,
    total_size: 9,
    parameters: [
      {
        name: "mode_state_number",
        type: "int",
        size: 1,
      },
      {
        name: "time",
        type: "int",
        size: 7,
      }
    ]
  },
];

export { mockCommandsList };