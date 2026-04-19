import { createColumnHelper } from "@tanstack/react-table";
import Table from "../components/Table";

type DashboardData = {
  session: string;
  type: string;
  timestamp: string;
  packet: string;
};

const columnHelper = createColumnHelper<DashboardData>();

const columns = [
  columnHelper.accessor("session", {
    header: "Session",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("type", {
    header: "Type",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("timestamp", {
    header: "Timestamp",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("packet", {
    header: "Packet",
    cell: (info) => info.getValue(),
  }),
];

// Demo data
const data: DashboardData[] = [
  {
    session: "Session 1",
    type: "Telemetry",
    timestamp: "2025-10-11 14:23:45",
    packet: "0x4A2F",
  },
  {
    session: "Session 2",
    type: "Command",
    timestamp: "2025-10-11 14:25:12",
    packet: "0x3B1E",
  },
  {
    session: "Session 3",
    type: "Telemetry",
    timestamp: "2025-10-11 14:27:33",
    packet: "0x5C4D",
  },
  {
    session: "Session 1",
    type: "Status",
    timestamp: "2025-10-11 14:30:01",
    packet: "0x6D5E",
  },
];

/**
 * @brief Dashboard component displaying the dashboard table
 * @return tsx element of Dashboard component
 */
function Dashboard() {
  return (
    <div className="flex flex-col items-center justify-center min-h-screen px-4 pt-32">
      <Table data={data} columns={columns} showFilters={true} />
    </div>
  );
}

export default Dashboard;
