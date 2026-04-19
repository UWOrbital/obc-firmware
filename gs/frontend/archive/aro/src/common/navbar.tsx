import logo from "../assets/orbital_logo.png";
import { Link } from "react-router-dom";
import profile from "../assets/profile.png";

function Navbar() {
  return (
    <div className="nav">
      <Link to="/">
        <img src={logo} alt="orbital logo"></img>
      </Link>
      <div className="navmenu">
        <Link to="/">Home</Link>
        <Link to="/new-request">New</Link>
        <Link to="/requests">Requests</Link>
        <Link to="/profile">
          <div className="profile">
            <img src={profile} alt="profile"></img>
          </div>
        </Link>
      </div>
    </div>
  );
}

export default Navbar;
