import { type MouseEvent, useEffect, useState, useMemo } from "react";
import type { RequestItemData, AROCommandStatus } from "./request-item-data.ts";
import { getRequestItems } from "./requests-api.ts";
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

const Requests = () => {
  // TODO: Switch to using react-query
  const [data, setData] = useState<RequestItemData[]>([]);
  const [sorting, setSorting] = useState<SortingState>([]);
  const [columnFilters, setColumnFilters] = useState<ColumnFiltersState>([]);

  useEffect(() => {
    const getRequestItemsRegular = async () => {
      const response = await getRequestItems();
      setData(response);
    };
    getRequestItemsRegular();
  }, []);

  // Removes the request with the given id from the list of data
  // TODO: Cancel request on the backend
  const cancelRequest = (id: number) => {
    return async (a: MouseEvent) => {
      a.preventDefault();
      setData((prev: RequestItemData[]) =>
        prev.filter((item) => item.id != id)
      );
    };
  };

  const columnHelper = createColumnHelper<RequestItemData>();

  const columns = useMemo(() => [
    columnHelper.accessor("id", {
      header: "ID",
      cell: (info) => info.getValue(),
    }),
    columnHelper.accessor("latitude", {
      header: "Latitude",
      cell: (info) => info.getValue(),
    }),
    columnHelper.accessor("longitude", {
      header: "Longitude",
      cell: (info) => info.getValue(),
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
    columnHelper.accessor("created_on", {
      header: "Created On",
      cell: (info) => info.getValue().toString(),
    }),
    columnHelper.accessor("request_sent_to_obc_on", {
      header: "Request Sent To OBC On",
      cell: (info) => info.getValue()?.toString() || "",
    }),
    columnHelper.accessor("pic_taken_on", {
      header: "Picture Taken On",
      cell: (info) => info.getValue()?.toString() || "",
    }),
    columnHelper.accessor("pic_transmitted_on", {
      header: "Picture Transmitted On",
      cell: (info) => info.getValue()?.toString() || "",
    }),
    columnHelper.display({
      id: "downloadPacket",
      header: "Download Packet",
      cell: ({ row }) => (
        <button onClick={async (a) => {
          a.preventDefault();
          console.log(`Downloading packet for ${row.original.id}`);
        }}>
          Download Packet {row.original.id}
        </button>
      ),
    }),
    columnHelper.display({
      id: "cancelRequest",
      header: "Cancel Request",
      cell: ({ row }) => (
        <button
          disabled={new Date() < row.original.cancellable_after}
          onClick={cancelRequest(row.original.id)}
        >
          Cancel Request {row.original.id}
        </button>
      ),
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

  if (data.length === 0) {
    return <div>You do not have any request created.</div>;
  }

  return (
    <div>
      <div style={{ marginBottom: "1rem" }}>
        <input
          placeholder="Filter by status..."
          value={table.getColumn("status")?.getFilterValue() as string || ""}
          onChange={e => table.getColumn("status")?.setFilterValue(e.target.value)}
          style={{ marginRight: "1rem" }}
        />
      </div>
      <table style={{ width: "100%", borderCollapse: "collapse" }}>
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
                  {header.column.getIsSorted() === "asc" ? " ↑ " : ""}
                  {header.column.getIsSorted() === "desc" ? " ↓ " : ""}
                </th>
              ))}
            </tr>
          ))}
        </thead>
        <tbody>
          {table.getRowModel().rows.map(row => (
            <tr key={row.id}>
              {row.getVisibleCells().map(cell => (
                <td key={cell.id}>
                  {flexRender(cell.column.columnDef.cell, cell.getContext())}
                </td>
              ))}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
};

export default Requests;