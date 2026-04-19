import { render, screen } from "@testing-library/react";
import { describe, it, expect, vi } from "vitest";
import { createColumnHelper } from "@tanstack/react-table";
import Table from "./Table";

type TestData = {
  id: number;
  name: string;
  status: string;
};

const columnHelper = createColumnHelper<TestData>();

const columns = [
  columnHelper.accessor("id", {
    header: "ID",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("name", {
    header: "Name",
    cell: (info) => info.getValue(),
  }),
  columnHelper.accessor("status", {
    header: "Status",
    cell: (info) => info.getValue(),
  }),
];

const testData: TestData[] = [
  { id: 1, name: "Test 1", status: "Active" },
  { id: 2, name: "Test 2", status: "Inactive" },
];

describe("Table Component", () => {
  it("renders table with data", () => {
    render(<Table data={testData} columns={columns} />);

    expect(screen.getByText("ID")).toBeInTheDocument();
    expect(screen.getByText("Name")).toBeInTheDocument();
    expect(screen.getByText("Status")).toBeInTheDocument();
    expect(screen.getByText("Test 1")).toBeInTheDocument();
    expect(screen.getByText("Test 2")).toBeInTheDocument();
  });

  it("renders filter inputs when showFilters is true", () => {
    render(<Table data={testData} columns={columns} showFilters={true} />);

    const filterInputs = screen.getAllByRole("textbox");
    expect(filterInputs).toHaveLength(1);
    expect(filterInputs[0]).toHaveAttribute(
      "placeholder",
      "Search..."
    );
  });

  it("does not render filter inputs when showFilters is false", () => {
    render(<Table data={testData} columns={columns} showFilters={false} />);

    const filterInputs = screen.queryAllByRole("textbox");
    expect(filterInputs).toHaveLength(0);
  });

  it("calls onRowClick when row is clicked", () => {
    const handleRowClick = vi.fn();
    render(
      <Table data={testData} columns={columns} onRowClick={handleRowClick} />
    );

    const firstRow = screen.getByText("Test 1").closest("tr");
    firstRow?.click();

    expect(handleRowClick).toHaveBeenCalledWith(testData[0]);
  });

  it("applies custom containerClassName", () => {
    const customClass = "custom-container-class";
    const { container } = render(
      <Table
        data={testData}
        columns={columns}
        containerClassName={customClass}
      />
    );

    const tableContainer = container.querySelector(`.${customClass}`);
    expect(tableContainer).toBeInTheDocument();
  });
});
