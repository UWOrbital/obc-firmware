import { describe, it, expect } from "vitest";
import { render, screen } from "@testing-library/react";
import "@testing-library/jest-dom";
import Background from "./Background";

describe("Background", () => {
  it("renders background image", () => {
    render(<Background />);
    expect(screen.getByAltText("background-image")).toBeInTheDocument();
  });

  it("has correct CSS classes", () => {
    render(<Background />);
    const image = screen.getByAltText("background-image");
    expect(image).toHaveClass("fixed inset-0 w-full h-full object-cover -z-10");
  });
});
