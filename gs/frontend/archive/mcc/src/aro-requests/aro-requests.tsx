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


import { type AROCommandStatus } from "../shared-types.ts";

type AROItemProps = {
  id: number;
  aro_id?: number;
  latitude: number;
  longitude: number;
  status: AROCommandStatus;
  created_on?: string;
  request_sent_to_obc_on?: string | null;
  pic_taken_on?: string | null;
  pic_transmitted_on?: string | null;
  packet_id?: number | null;
};

function ARORequests() {
  const [loading, setLoading] = useState(true);
  const [aroRequests, setARORequests] = useState<AROItemProps[]>([]);
  const [sorting, setSorting] = useState<SortingState>([]);
  const [columnFilters, setColumnFilters] = useState<ColumnFiltersState>([]);

  useEffect(() => {
    fetch("http://localhost:5000/aro-request")
      .then((response) => response.json())
      .then((data) => {
        setARORequests(data);
        setLoading(false);
      });
  }, []);

  const columnHelper = createColumnHelper<AROItemProps>();

  const columns = useMemo(() => [
    columnHelper.accessor("id", {
      header: "#",
      cell: (info) => info.getValue(),
    }),
    columnHelper.accessor("latitude", {
      header: "Latitude",
      cell: (info) => info.getValue().toFixed(6),
    }),
    columnHelper.accessor("longitude", {
      header: "Longitude",
      cell: (info) => info.getValue().toFixed(6),
    }),
    columnHelper.accessor("status", {
      header: "Status",
      cell: (info) => {
        const status = info.getValue();

        const statusDisplay = status.charAt(0).toUpperCase() + status.slice(1);
        const getStatusColor = (status: AROCommandStatus) => {
          switch (status) {
            case 'pending': return '#ffc107';
            case 'scheduled': return '#007bff';
            case 'taken': return '#28a745';
            case 'cancelled': return '#6c757d';
            case 'failed': return '#dc3545';
            case 'completed': return '#28a745';
            default: return '#000';
          }
        };
        return (
          <span style={{ color: getStatusColor(status), fontWeight: 'bold' }}>
            {statusDisplay}
          </span>
        );
      },
    }),
    ...(aroRequests.some(req => req.created_on) ? [
      columnHelper.accessor("created_on", {
        header: "Created On",
        cell: (info) => info.getValue() ? new Date(info.getValue()!).toLocaleString() : '',
      })
    ] : []),
  ], [aroRequests]);

  const table = useReactTable({
    data: aroRequests,
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
    <div className="arorequests layout">
      <div style={{ marginBottom: "1rem" }}>
        <input
          placeholder="Filter by status..."
          value={table.getColumn("status")?.getFilterValue() as string || ""}
          onChange={e => table.getColumn("status")?.setFilterValue(e.target.value)}
          style={{ marginRight: "1rem" }}
        />
        <small style={{ color: '#6c757d' }}>
          Available statuses: pending, scheduled, taken, cancelled, failed, completed
        </small>
      </div>

      <Table striped bordered hover variant="light">
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
              <td colSpan={4}>Loading...</td>
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

export default ARORequests;