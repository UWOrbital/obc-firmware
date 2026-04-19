import React from "react";
import ReactDOM from "react-dom/client";
import App from "./App.tsx";
import "./index.css"; // default styles are applied here
import { BrowserRouter } from "react-router-dom";

/**
 * @brief Main component displaying the main application, which is linked to the index.html file
 * @return tsx element of Main component
 */
ReactDOM.createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
      <BrowserRouter>
        <App />
      </BrowserRouter>
  </React.StrictMode>
);
