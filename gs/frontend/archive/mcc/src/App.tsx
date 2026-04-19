import { BrowserRouter, Route, Routes } from "react-router-dom";
import ARORequests from "./aro-requests/aro-requests.tsx";
import Header from "./common/header.tsx";
import Logs from "./common/logs.tsx";
import NotFound from "./common/not-found.tsx";
import Dashboard from "./dashboard/dashboard.tsx";
import MissionCommands from "./mission-commands/mission-commands.tsx";
import TelemetryData from "./telemetry/telemetry-data.tsx";

function App() {
  return (
    <BrowserRouter>
      <Header />
      <Routes>
        <Route path="*" element={<NotFound />} />
        <Route path="/" element={<Dashboard />} />
        <Route path="/mission-commands" element={<MissionCommands />} />
        <Route path="/telemetry-data" element={<TelemetryData />} />
        <Route path="/aro-requests" element={<ARORequests />} />
      </Routes>
      <Logs />
    </BrowserRouter>
  );
}

export default App;
