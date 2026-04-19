import {
  useReactTable,
  getCoreRowModel,
  flexRender,
  createColumnHelper,
  getSortedRowModel,
  getFilteredRowModel,
} from "@tanstack/react-table";
import { useState } from "react";

type SessionData = {
  id: string;
  operator: string;
  startTime: string;
  duration: string;
  status: "Active" | "Paused" | "Completed";
};

const columnHelper = createColumnHelper<SessionData>();

const columns = [
  columnHelper.accessor("id", {
    header: "Session ID",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("operator", {
    header: "Operator",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("startTime", {
    header: "Start Time",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("duration", {
    header: "Duration",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("status", {
    header: "Status",
    cell: (info) => {
      const status = info.getValue();
      const statusColors: Record<string, string> = {
        Active: "text-green-400",
        Paused: "text-yellow-400",
        Completed: "text-blue-400",
      };
      return (
        <span className={statusColors[status] || "text-gray-400"}>
          {status}
        </span>
      );
    },
  }),
];

// Demo data
const data: SessionData[] = [
  {
    id: "SES-001",
    operator: "John Smith",
    startTime: "2025-10-11 13:00:00",
    duration: "1h 23m",
    status: "Active",
  },
  {
    id: "SES-002",
    operator: "Jane Doe",
    startTime: "2025-10-11 12:30:00",
    duration: "53m",
    status: "Paused",
  },
  {
    id: "SES-003",
    operator: "Bob Johnson",
    startTime: "2025-10-11 11:00:00",
    duration: "2h 15m",
    status: "Completed",
  },
  {
    id: "SES-004",
    operator: "Alice Williams",
    startTime: "2025-10-11 14:20:00",
    duration: "12m",
    status: "Active",
  },
];

/**
 * @brief LiveSession component displaying the live sessions table
 * @return tsx element of LiveSession component
 */
function LiveSession() {
  const [columnFilters, setColumnFilters] = useState("");
  const [globalFilter, setGlobalFilter] = useState("");

  const table = useReactTable({
    data,
    columns,
    getCoreRowModel: getCoreRowModel(),
    getSortedRowModel: getSortedRowModel(),
    getFilteredRowModel: getFilteredRowModel(),
    state: {
      globalFilter,
    },
    onGlobalFilterChange: setGlobalFilter,
  });

  return (
    <div className="flex flex-col items-center justify-center min-h-screen px-4 pt-32">
      <div className="w-full max-w-5xl">
        {/* Filter Section */}
        <div className="flex justify-between mb-6 gap-4">
          <input
            type="text"
            placeholder="Filter a column..."
            value={columnFilters}
            onChange={(e) => setColumnFilters(e.target.value)}
            className="flex-1 bg-gray-800/50 text-white px-4 py-2 rounded-lg border border-gray-600/50 focus:outline-none focus:border-gray-500"
          />
          <input
            type="text"
            placeholder="Filtered item name"
            value={globalFilter ?? ""}
            onChange={(e) => setGlobalFilter(e.target.value)}
            className="flex-1 bg-gray-800/50 text-white px-4 py-2 rounded-lg border border-gray-600/50 focus:outline-none focus:border-gray-500"
          />
        </div>

        {/* Table */}
        <div className="bg-gray-900/30 backdrop-blur-sm rounded-lg overflow-hidden border border-gray-700/50">
          <table className="w-full text-white">
            <thead>
              {table.getHeaderGroups().map((headerGroup) => (
                <tr
                  key={headerGroup.id}
                  className="border-b border-gray-700/50"
                >
                  {headerGroup.headers.map((header) => (
                    <th
                      key={header.id}
                      className="text-left px-6 py-4 font-normal text-gray-300"
                    >
                      {header.isPlaceholder
                        ? null
                        : flexRender(
                            header.column.columnDef.header,
                            header.getContext()
                          )}
                    </th>
                  ))}
                </tr>
              ))}
            </thead>
            <tbody>
              {table.getRowModel().rows.map((row) => (
                <tr
                  key={row.id}
                  className="border-b border-gray-800/50 hover:bg-gray-800/30 transition-colors"
                >
                  {row.getVisibleCells().map((cell) => (
                    <td key={cell.id} className="px-6 py-4 text-gray-200">
                      {flexRender(
                        cell.column.columnDef.cell,
                        cell.getContext()
                      )}
                    </td>
                  ))}
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}

export default LiveSession;
