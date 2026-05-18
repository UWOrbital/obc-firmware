import { useState, useRef, useEffect } from "react";
import {
    createColumnHelper,
    type ColumnFiltersState,
    type ExpandedState,
    flexRender,
    getCoreRowModel,
    getExpandedRowModel,
    getFilteredRowModel,
    getSortedRowModel,
    type SortingState,
    useReactTable
} from "@tanstack/react-table";

import rawData from "../utils/telemetry-mock-data.json"

type telemetryData = {
    type?: string,
    timestamp?: string,
    value?: number,
    id?: number,
    session?: string,
    packet?: string,
    obc_state?: string,
    epc_state?: string,
    subRows?: telemetryData[],
};

const uniqueTypes = Array.from(new Set(rawData.map(row => row.type))).sort();

const data: telemetryData[] = rawData;

const columnHelper = createColumnHelper<telemetryData>();

const columns = [
    columnHelper.accessor("type", {
        id: "type",
        header: "Type",
        enableSorting: false,
        filterFn: (row, _columnId, filterValue) => {
            if (!filterValue) return true;
            if (row.depth > 0) return true;
            return row.original.type === filterValue;
        },
        cell: (info) => {
            const row = info.row.original;
            const isChild = info.row.depth > 0;
            const statusColors: Record<string, string> = {
                Pending: "text-yellow-400",
                Sent: "text-green-400",
                Received: "text-blue-400",
                Failed: "text-red-400",
            };
            return (
                <div
                    style={{ paddingLeft: `${info.row.depth * 1}rem` }}
                    className="flex items-center gap-1"
                >
                    {info.row.getCanExpand() ? (
                        <button onClick={info.row.getToggleExpandedHandler()}>
                            {info.row.getIsExpanded() ? "▼ " : "▶ "}
                        </button>
                    ) : (
                        <span className="w-3" />
                    )}
                    {isChild
                        ? (
                            <div className="border-b-2 border-t-2 border-[#898989] py-2 text-left w-full">
                                <p>ID: {row.id}</p>
                                <p>{row.session}</p>
                                <p>Packet: {row.packet}</p>
                                <p>OBC_State: <span className={statusColors[row.obc_state ?? ""] ?? "text-gray-400"}>{row.obc_state}</span></p>
                                <p>EPS_State: <span className={statusColors[row.epc_state ?? ""] ?? "text-gray-400"}>{row.epc_state}</span></p>
                            </div>
                        )
                        : row.type}
                </div>
            );
        },
    }),
    columnHelper.accessor("timestamp", {
        header: "Timestamp",
        cell: (info) => info.row.original.timestamp,
        sortingFn: "alphanumeric",
    }),
    columnHelper.accessor("value", {
        header: "Value",
        cell: (info) => info.row.original.value,
        sortingFn: "basic",
    }),
];

function Telemetry() {
    const type: string = "< log >";
    const [sorting, setSorting] = useState<SortingState>([]);
    const [columnFilters, setColumnFilters] = useState<ColumnFiltersState>([]);
    const [globalFilter, setGlobalFilter] = useState("");
    const [selectedRowId, setSelectedRowId] = useState<string | null>(null);
    const rowRefs = useRef<Map<string, HTMLTableRowElement>>(new Map());
    const scrollRef = useRef<HTMLDivElement>(null);
    const [expanded, setExpanded] = useState<ExpandedState>({});

    const table = useReactTable({
        data,
        columns,
        state: { sorting, expanded, columnFilters, globalFilter },
        onSortingChange: setSorting,
        onExpandedChange: setExpanded,
        onColumnFiltersChange: setColumnFilters,
        onGlobalFilterChange: setGlobalFilter,
        getSubRows: (row) => row.subRows,
        getCoreRowModel: getCoreRowModel(),
        getSortedRowModel: getSortedRowModel(),
        getExpandedRowModel: getExpandedRowModel(),
        getFilteredRowModel: getFilteredRowModel(),
        globalFilterFn: (row, _columnId, filterValue) => {
            const search = (filterValue as string).toLowerCase();

            const matchesRow = (r: telemetryData): boolean => {
                return [
                    r.type, r.timestamp, String(r.value ?? ""),
                    r.session, r.packet, r.obc_state, r.epc_state, String(r.id ?? "")
                ].some(val => val?.toLowerCase().includes(search));
            };

            return matchesRow(row.original) || (row.original.subRows?.some(matchesRow) ?? false);
        },
    });

    const rows = table.getRowModel().rows;

    useEffect(() => {
        if (selectedRowId === null) return;
        const row = rowRefs.current.get(selectedRowId);
        const container = scrollRef.current;
        if (!row || !container) return;

        const rowTop = row.offsetTop;
        const rowBottom = rowTop + row.offsetHeight;
        const visibleTop = container.scrollTop;
        const visibleBottom = visibleTop + container.clientHeight;
        const theadHeight = container.querySelector("thead")?.offsetHeight ?? 0;

        if (rowTop < visibleTop + theadHeight) {
            container.scrollTop = rowTop - theadHeight;
        } else if (rowBottom > visibleBottom) {
            container.scrollTop = rowBottom - container.clientHeight;
        }
    }, [selectedRowId]);

    return (
        <div>
            <div
                id="outline"
                className="border-2 border-[#898989] rounded-2xl py-2 px-3 h-[88vh] flex flex-col overflow-hidden"
            >
                <div id="header" className="flex flex-row w-full gap-4 items-center">
                    <span className="italic font-bold">{type}</span>
                </div>

                {/* Search and Filter inputs */}
                <div className="flex flex-row gap-2 py-1">
                    <input
                        type="text"
                        placeholder="Search..."
                        className="bg-transparent border border-[#898989] rounded px-2 py-1 text-sm outline-none flex-1"
                        value={globalFilter}
                        onChange={(e) => setGlobalFilter(e.target.value)}
                    />
                    <select
                        className="bg-black border border-[#898989] rounded px-2 py-1 text-xs outline-none flex-1"
                        value={(table.getColumn("type")?.getFilterValue() as string) ?? ""}
                        onChange={(e) => table.getColumn("type")?.setFilterValue(e.target.value || undefined)}
                    >
                        <option value="">All types</option>
                        {uniqueTypes.map(type => (
                            <option key={type} value={type}>{type}</option>
                        ))}
                    </select>
                </div>

                <div
                    ref={scrollRef}
                    className="overflow-y-auto flex-1 outline-none"
                    tabIndex={0}
                    onKeyDown={(e) => {
                        if (e.key === "ArrowDown") {
                            e.preventDefault();
                            if (selectedRowId === null) {
                                setSelectedRowId(rows[0]?.id ?? null);
                            } else {
                                const currentIdx = rows.findIndex(r => r.id === selectedRowId);
                                const next = rows[Math.min(currentIdx + 1, rows.length - 1)];
                                setSelectedRowId(next?.id ?? null);
                            }
                        } else if (e.key === "ArrowUp") {
                            e.preventDefault();
                            if (selectedRowId === null) {
                                setSelectedRowId(rows[0]?.id ?? null);
                            } else {
                                const currentIdx = rows.findIndex(r => r.id === selectedRowId);
                                const prev = rows[Math.max(currentIdx - 1, 0)];
                                setSelectedRowId(prev?.id ?? null);
                            }
                        }
                    }}
                >
                    <table className="w-full">
                        <thead className="sticky top-0 bg-black z-10">
                            <tr className="flex flex-row border-b-2 border-[#898989] gap-2 text-center pb-1">
                                <td className="flex flex-row justify-between pl-2 w-1/2">
                                    {table.getFlatHeaders()
                                        .filter(header => ["type", "timestamp"].includes(header.id))
                                        .map(header => (
                                            <th
                                                key={header.id}
                                                className={`font-normal text-center w-1/2 ${header.column.getCanSort() ? "cursor-pointer select-none" : ""}`}
                                                onClick={header.column.getToggleSortingHandler()}
                                            >
                                                {flexRender(header.column.columnDef.header, header.getContext())}
                                                {{ asc: " ▲", desc: " ▼" }[header.column.getIsSorted() as string] ?? null}
                                            </th>
                                        ))}
                                </td>
                                {table.getFlatHeaders()
                                    .filter(header => header.id === "value")
                                    .map(header => (
                                        <th
                                            key={header.id}
                                            className={`flex-1 font-normal ${header.column.getCanSort() ? "cursor-pointer select-none" : ""}`}
                                            onClick={header.column.getToggleSortingHandler()}
                                        >
                                            {flexRender(header.column.columnDef.header, header.getContext())}
                                            {{ asc: " ▲", desc: " ▼" }[header.column.getIsSorted() as string] ?? null}
                                        </th>
                                    ))}
                            </tr>
                        </thead>
                        <tbody>
                            {rows.map((row) => (
                                <tr
                                    key={row.id}
                                    ref={(el) => {
                                        if (el) rowRefs.current.set(row.id, el);
                                        else rowRefs.current.delete(row.id);
                                    }}
                                    className={`flex flex-row gap-2 cursor-pointer ${
                                        selectedRowId === row.id ? "bg-white text-[#1C1F1B]" : ""
                                    }`}
                                    onClick={() => setSelectedRowId(row.id)}
                                >
                                    <td className="flex flex-row justify-between pl-2 w-1/2 text-center">
                                        {row.getVisibleCells()
                                            .filter(cell => ["type", "timestamp"].includes(cell.column.id))
                                            .map(cell => (
                                                <span key={cell.id} className="w-1/2">
                                                    {flexRender(cell.column.columnDef.cell, cell.getContext())}
                                                </span>
                                            ))}
                                    </td>
                                    {row.getVisibleCells()
                                        .filter(cell => cell.column.id === "value")
                                        .map(cell => (
                                            <td key={cell.id} className="flex-1 text-center">
                                                {flexRender(cell.column.columnDef.cell, cell.getContext())}
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

export default Telemetry;