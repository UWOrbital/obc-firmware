// Mock telemetry data for testing and developing the telemetry page.
// This includes telemetry types, subtypes, and sample data points.
// Currently, format of this data may not match real telemetry data structure - refactor as needed.

export const telemetryTypes = ["Current", "Voltage", "Motor"];

export const currentSubtypes = ["3v3", "5v", "12v"];
export const voltageSubtypes = ["3v3", "5v", "7v"];
export const motorSubtypes = ["Motor A", "Motor B", "Motor C"];

export const telemetrySubtypes: { [key: string]: string[] } = {
  Current: currentSubtypes,
  Voltage: voltageSubtypes,
  Motor: motorSubtypes,
};


export type TelemetryDataType = {
  session: string;
  telemetry: string;
  type: string; // e.g. '3v3', '5v', 'Motor A'
  packet: string;
  datapoints: Array<{ timestamp: number; value: number }>;
};


export const telemetryData: { [type: string]: TelemetryDataType[] } = {
  Current: [
    {
      session: "Session A",
      telemetry: "Telemetry 1",
      type: "3v3",
      packet: "Packet Data 1",
      datapoints: [
        { timestamp: 1, value: 0.5 },
        { timestamp: 2, value: 0.7 },
        { timestamp: 3, value: 0.6 },
        { timestamp: 4, value: 0.8 },
      ],
    },
    {
      session: "Session B",
      telemetry: "Telemetry 2",
      type: "5v",
      packet: "Packet Data 2",
      datapoints: [
        { timestamp: 1, value: 1.2 },
        { timestamp: 2, value: 1.1 },
        { timestamp: 3, value: 1.3 },
        { timestamp: 4, value: 1.4 },
      ],
    },
    {
      session: "Session C",
      telemetry: "Telemetry 3",
      type: "12v",
      packet: "Packet Data 3",
      datapoints: [
        { timestamp: 1, value: 2.0 },
        { timestamp: 2, value: 2.1 },
        { timestamp: 3, value: 2.2 },
        { timestamp: 4, value: 2.3 },
      ],
    },
  ],
  Voltage: [
    {
      session: "Session D",
      telemetry: "Telemetry 4",
      type: "3v3",
      packet: "Packet Data 4",
      datapoints: [
        { timestamp: 1, value: 3.3 },
        { timestamp: 2, value: 3.2 },
        { timestamp: 3, value: 3.4 },
        { timestamp: 4, value: 3.3 },
      ],
    },
    {
      session: "Session E",
      telemetry: "Telemetry 5",
      type: "5v",
      packet: "Packet Data 5",
      datapoints: [
        { timestamp: 1, value: 5.0 },
        { timestamp: 2, value: 5.1 },
        { timestamp: 3, value: 5.0 },
        { timestamp: 4, value: 5.2 },
      ],
    },
    {
      session: "Session F",
      telemetry: "Telemetry 6",
      type: "7v",
      packet: "Packet Data 6",
      datapoints: [
        { timestamp: 1, value: 7.0 },
        { timestamp: 2, value: 7.1 },
        { timestamp: 3, value: 7.2 },
        { timestamp: 4, value: 7.1 },
      ],
    },
  ],
  Motor: [
    {
      session: "Session G",
      telemetry: "Telemetry 7",
      type: "Motor A",
      packet: "Packet Data 7",
      datapoints: [
        { timestamp: 1, value: 100 },
        { timestamp: 2, value: 110 },
        { timestamp: 3, value: 120 },
        { timestamp: 4, value: 130 },
      ],
    },
    {
      session: "Session H",
      telemetry: "Telemetry 8",
      type: "Motor B",
      packet: "Packet Data 8",
      datapoints: [
        { timestamp: 1, value: 90 },
        { timestamp: 2, value: 95 },
        { timestamp: 3, value: 92 },
        { timestamp: 4, value: 98 },
      ],
    },
    {
      session: "Session I",
      telemetry: "Telemetry 9",
      type: "Motor C",
      packet: "Packet Data 9",
      datapoints: [
        { timestamp: 1, value: 80 },
        { timestamp: 2, value: 85 },
        { timestamp: 3, value: 88 },
        { timestamp: 4, value: 87 },
      ],
    },
  ],
};
