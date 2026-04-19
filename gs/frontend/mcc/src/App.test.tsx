import { describe, it } from "vitest";
import { render, screen } from "@testing-library/react";
import { BrowserRouter } from "react-router-dom";
import App from "./App";

describe("App", () => {
  it("should render without crashing", () => {
    render(
      <BrowserRouter>
        <App />
      </BrowserRouter>
    );

    screen.debug(); // prints out the jsx in the App component unto the command line
  });
});
