import { Link } from "react-router-dom";
import logo from "../assets/logo.png";
import "./header.css";

function Header() {
  return (
    <div className="nav">
      <Link to="/">
        <img src={logo} alt="orbital logo"></img>
      </Link>
      <div className="navmenu">
        <Link to="/">Dashboard</Link>
        <Link to="/mission-commands">Mission Commands</Link>
        <Link to="/telemetry-data">Telemetry Data</Link>
        <Link to="/aro-requests">ARO Requests</Link>
      </div>
    </div>
  );
}

export default Header;
