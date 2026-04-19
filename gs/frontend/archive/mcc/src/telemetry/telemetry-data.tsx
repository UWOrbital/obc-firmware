import Table from "react-bootstrap/Table";
import { useState, useMemo, useEffect } from "react";
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
import { type TelemetryWithMaster } from "../shared-types.ts";


function TelemetryData() {
  const [data, setData] = useState<TelemetryWithMaster[]>([]);
  const [loading, setLoading] = useState(true);
  const [sorting, setSorting] = useState<SortingState>([]);
  const [columnFilters, setColumnFilters] = useState<ColumnFiltersState>([]);

  useEffect(() => {
    const fetchTelemetryData = async () => {
      try {
        setLoading(true);
        const response = await fetch('http://localhost:5000/telemetry');

        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }

        const telemetryData = await response.json();
        setData(telemetryData);
      } catch (error) {
        console.error('Error fetching telemetry data:', error);

        setData([]);
      } finally {
        setLoading(false);
      }
    };

    fetchTelemetryData();


    const interval = setInterval(fetchTelemetryData, 30000); // Future code reader: This is optional btw and can be tweaked based on what you want
    return () => clearInterval(interval);
  }, []);

  const columnHelper = createColumnHelper<TelemetryWithMaster>();

  const columns = useMemo(() => [
    columnHelper.accessor("id", {
      header: "ID",
      cell: (info) => info.getValue(),
    }),
    columnHelper.accessor("telemetry_name", {
      header: "Type",
      cell: (info) => info.getValue() || `Type ${info.row.original.type}`,
    }),
    columnHelper.accessor("value", {
      header: "Value",
      cell: (info) => info.getValue(),
    }),
    columnHelper.accessor("telemetry_format", {
      header: "Format",
      cell: (info) => info.getValue() || "Unknown",
    }),
    columnHelper.accessor("created_on", {
      header: "Timestamp",
      cell: (info) => info.getValue() ? new Date(info.getValue()!).toLocaleString() : "N/A",
    }),
  ], []);

  const table = useReactTable({
    data,
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
    <div className="telemetryData layout">
      <div style={{ marginBottom: "1rem" }}>
        <input
          placeholder="Filter by telemetry type..."
          value={table.getColumn("telemetry_name")?.getFilterValue() as string || ""}
          onChange={e => table.getColumn("telemetry_name")?.setFilterValue(e.target.value)}
          style={{ marginRight: "1rem" }}
        />
        <input
          placeholder="Filter by value..."
          value={table.getColumn("value")?.getFilterValue() as string || ""}
          onChange={e => table.getColumn("value")?.setFilterValue(e.target.value)}
        />
      </div>

      <Table striped bordered hover>
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
              <td colSpan={5}>Loading telemetry data...</td>
            </tr>
          ) : data.length === 0 ? (
            <tr>
              <td colSpan={5}>No telemetry data available</td>
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

export default TelemetryData;