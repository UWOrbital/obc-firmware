import { Route, Routes } from "react-router-dom";
import { ToastContainer } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import Nav from "./components/Nav";
import Background from "./components/Background";
import NewRequestForm from "./components/new-request/new-request-form.tsx";
import Login from "./components/auth/login.tsx"
import Signup from "./components/auth/sign-up.tsx"
import Verify from "./components/auth/verify.tsx"
import Home from "./components/Home.tsx"
import PageNotFound from "./components/PageNotFound.tsx";
import Gallery from "./components/profile/photo-gallery/gallery.tsx";
import ProfileForm from "./components/profile/profile-form/profile-form.tsx";

/**
 * @brief App component displaying the main application
 * @return tsx element of App component
 */
function App() {
  return (
    <>
      <Nav />
      <Background />
      <Routes>
        {/* uncomment these routes as the pages are created */}
        {/* <Route path="*" element={<Dashboard />} />
        <Route path="/commands" element={<Commands />} />
        <Route path="/new-request" element={<NewRequestForm />} />
        <Route path="/requests" element={<Requests />} />
        <Route path="/profile" element={<Profile />} />*/}
        <Route path="/" element={<Home/>} />
        <Route path="/login" element={<Login />} />
        <Route path="/verify" element={<Verify />} />
        <Route path="/sign-up" element={<Signup />} />
        <Route path="/new-request" element={<NewRequestForm />} />
        <Route path="*" element={<PageNotFound />} />
        <Route path="/profile/gallery" element={<Gallery />} />
        <Route path="/profile/settings" element={<ProfileForm />} />
      </Routes>
      <ToastContainer />
    </>
  );
}

export default App;
