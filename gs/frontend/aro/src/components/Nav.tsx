import { Link } from "react-router-dom";
import orbital_logo from "../assets/orbital_logo.png";
// import { CSS_VARIABLES } from "../utils/themes";
import { NAVIGATION_LINKS } from "../utils/routes";

/**
 * @brief Nav component displaying the navigation bar
 * @return tsx element of Nav component
 */
function Nav() {
  // TODO: create user auth that checks if the user is logged in
  const isLoggedIn = false;

  return (
    <nav className="m-7 text-white">
      {/* Logo */}
      <div className="absolute left-14">
        <Link to="/" className="hover:opacity-80 transition-opacity">
          <img src={orbital_logo} alt="orbital-logo" className="h-12 w-auto" />
        </Link>
      </div>

      {/* Navigation Links */}
      {isLoggedIn? (
        <div className="absolute right-14 mt-2 flex space-x-7">
          {NAVIGATION_LINKS.map((link) => (
            <Link key={link.url} to={link.url} className="mt-1 hover:underline">
              {link.text}
            </Link>
          ))}
          <div className="border-1 border-white rounded-xl p-1 px-2 hover:bg-white hover:text-black">
            <Link to="/profile" className="">
              Profile
            </Link>
          </div>
        </div>
      ) : (
        <div className="absolute right-14 mt-2 flex space-x-7">
          {NAVIGATION_LINKS.filter((link) => link.url !== "/new-request").map((link) => (
            <Link key={link.url} to={link.url} className="mt-1 hover:underline">
              {link.text}
            </Link>
          ))}
          <div className="border-1 border-white rounded-xl p-1 px-2 hover:bg-white hover:text-black">
            <Link to="/login" className="">
              Login
            </Link>
          </div>
        </div>
      )}
    </nav>
  );
}

export default Nav;
