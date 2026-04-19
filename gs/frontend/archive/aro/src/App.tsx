import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import { BrowserRouter, Route, Routes } from "react-router-dom";
import "./app.css";
import Navbar from "./common/navbar.tsx";
import NotFound from "./common/not-found.tsx";
import Home from "./home/home.tsx";
import NewRequestForm from "./new-request/new-request-form.tsx";
import Profile from "./profile/profile.tsx";
import Requests from "./requests/requests.tsx";

const queryClient = new QueryClient();

function App() {
  return (
    <QueryClientProvider client={queryClient}>
      <BrowserRouter>
        <Navbar />
        <Routes>
          <Route path="*" element={<NotFound />} />
          <Route path="/" element={<Home />} />
          <Route path="/new-request" element={<NewRequestForm />} />
          <Route path="/requests" element={<Requests />} />
          <Route path="/profile" element={<Profile />} />
        </Routes>
      </BrowserRouter>
    </QueryClientProvider>
  );
}

export default App;
