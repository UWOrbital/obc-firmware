import { describe, it, expect } from "vitest";
import { render, screen } from "@testing-library/react";
import "@testing-library/jest-dom";
import Background from "./Background";
import { ThemeProvider } from "../contexts/ThemeContext";

describe("Background", () => {
  it("renders background image", () => {
    render(
      <ThemeProvider>
        <Background />
      </ThemeProvider>
    );
    expect(screen.getByAltText("background-image")).toBeInTheDocument();
  });

  it("has correct CSS classes", () => {
    render(
      <ThemeProvider>
        <Background />
      </ThemeProvider>
    );
    const image = screen.getByAltText("background-image");
    // In light mode (default), opacity should be slightly higher for visibility
    expect(image).toHaveClass("opacity-30");
  });
});
