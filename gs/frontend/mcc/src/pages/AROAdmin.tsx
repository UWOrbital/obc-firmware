import { createColumnHelper } from "@tanstack/react-table";
import Table from "../components/Table";

type AROData = {
  session: string;
  request: string;
  status: string;
  priority: string;
  location: string;
};

const columnHelper = createColumnHelper<AROData>();

const columns = [
  columnHelper.accessor("session", {
    header: "Session",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("request", {
    header: "Request",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("status", {
    header: "Status",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("priority", {
    header: "Priority",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("location", {
    header: "Location",
    cell: (info) => info.getValue(),
  }),
];

// Demo data
const data: AROData[] = [
  {
    session: "Session 1",
    request: "Image Capture",
    status: "Pending",
    priority: "High",
    location: "47.5°N, 122.3°W",
  },
  {
    session: "Session 2",
    request: "Thermal Scan",
    status: "Processing",
    priority: "Medium",
    location: "34.0°N, 118.2°W",
  },
  {
    session: "Session 3",
    request: "Spectral Analysis",
    status: "Completed",
    priority: "Low",
    location: "40.7°N, 74.0°W",
  },
  {
    session: "Session 4",
    request: "Video Recording",
    status: "Failed",
    priority: "High",
    location: "51.5°N, 0.1°W",
  },
];

/**
 * @brief AROAdmin component displaying the ARO admin table
 * @return tsx element of AROAdmin component
 */
function AROAdmin() {
  return (
    <div className="flex flex-col items-center justify-center min-h-screen px-4 pt-32">
      <Table data={data} columns={columns} showFilters={true} />
    </div>
  );
}

export default AROAdmin;
