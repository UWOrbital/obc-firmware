import React from "react";
import ReactDOM from "react-dom/client";
import App from "./App.tsx";
import "./index.css"; // default styles are applied here
import { BrowserRouter } from "react-router-dom";
import { QueryClient, QueryClientProvider } from "@tanstack/react-query";

const queryClient = new QueryClient();
/**
 * @brief Main component displaying the main application, which is linked to the index.html file
 * @return tsx element of Main component
 */
ReactDOM.createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
    <BrowserRouter>
      <QueryClientProvider client={queryClient}>
        <App/>
      </QueryClientProvider>
    </BrowserRouter>
  </React.StrictMode>
);
