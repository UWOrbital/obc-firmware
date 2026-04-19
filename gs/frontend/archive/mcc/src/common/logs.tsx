import Table from "react-bootstrap/Table";
import { useEffect, useState, useMemo } from "react";
import {
  useReactTable,
  getCoreRowModel,
  getSortedRowModel,
  getFilteredRowModel,
  createColumnHelper,
  flexRender,
  type SortingState,
  type ColumnFiltersState
} from "@tanstack/react-table";

type LogData = {
  id: number;
  date: number;
  log: string;
};

function Logs() {
  const [logs, setLogs] = useState<LogData[]>([]);
  const [loading, setLoading] = useState(true);
  const [sorting, setSorting] = useState<SortingState>([]);
  const [columnFilters, setColumnFilters] = useState<ColumnFiltersState>([]);

  const fetchLogData = async () => {
    try {
      const response = await fetch(`http://localhost:5000/recent-logs/`);
      const data = await response.json();

      if (Array.isArray(data)) {
        setLogs(data.map((item, index) => ({ ...item, id: index })));
      } else {

        setLogs([{ id: 0, date: data.date, log: data.log }]);
      }
      setLoading(false);
    } catch (error) {
      console.error("Error fetching logs:", error);
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchLogData();
    const interval = setInterval(fetchLogData, 10000);
    return () => clearInterval(interval);
  }, []);

  const columnHelper = createColumnHelper<LogData>();

  const columns = useMemo(() => [
    columnHelper.accessor("date", {
      header: "Time",
      cell: (info) => info.getValue(),
    }),
    columnHelper.accessor("log", {
      header: "Log",
      cell: (info) => info.getValue(),
    }),
  ], []);

  const table = useReactTable({
    data: logs,
    columns,
    state: {
      sorting,
      columnFilters,
    },
    getCoreRowModel: getCoreRowModel(),
    getSortedRowModel: getSortedRowModel(),
    getFilteredRowModel: getFilteredRowModel(),
    onSortingChange: setSorting,
    onColumnFiltersChange: setColumnFilters,
  });

  return (
    <div className="logs layout">
      <div style={{ marginBottom: "1rem" }}>
        <input
          placeholder="Filter logs..."
          value={table.getColumn("log")?.getFilterValue() as string || ""}
          onChange={e => table.getColumn("log")?.setFilterValue(e.target.value)}
          style={{ marginRight: "1rem" }}
        />
      </div>

      <Table responsive="sm">
        <thead>
          {table.getHeaderGroups().map(headerGroup => (
            <tr key={headerGroup.id}>
              {headerGroup.headers.map(header => (
                <th
                  key={header.id}
                  style={{ cursor: header.column.getCanSort() ? 'pointer' : 'default' }}
                  onClick={header.column.getToggleSortingHandler()}
                >
                  {flexRender(
                    header.column.columnDef.header,
                    header.getContext()
                  )}
                  {header.column.getIsSorted() === "asc" ? " ↑" : ""}
                  {header.column.getIsSorted() === "desc" ? " ↓ " : ""}
                </th>
              ))}
            </tr>
          ))}
        </thead>
        <tbody>
          {loading ? (
            <tr>
              <td colSpan={2}>Loading...</td>
            </tr>
          ) : logs.length === 0 ? (
            <tr>
              <td colSpan={2}>No logs available</td>
            </tr>
          ) : (
            table.getRowModel().rows.map(row => (
              <tr key={row.id}>
                {row.getVisibleCells().map(cell => (
                  <td key={cell.id}>
                    {flexRender(cell.column.columnDef.cell, cell.getContext())}
                  </td>
                ))}
              </tr>
            ))
          )}
        </tbody>
      </Table>
    </div>
  );
}

export default Logs;