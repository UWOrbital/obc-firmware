import { Link } from "react-router-dom";
import orbital_logo from "../assets/orbital_logo.png";
import { NAVIGATION_LINKS } from "../utils/nav-links";
import { useTheme } from "../contexts/ThemeContext";

/**
 * @brief Nav component displaying the navigation bar
 * @return tsx element of Nav component
 */
function Nav() {
  // TODO: create user auth that checks if the user is logged in
  const isLoggedIn = false;
  const { theme, toggleTheme } = useTheme();

  return (
    <nav className="fixed top-0 left-0 right-0 h-16 px-8 flex items-center justify-between bg-background/80 backdrop-blur-sm border-b border-border z-50">
      {/* Logo */}
      <Link to="/" className="hover:opacity-80 transition-opacity">
        <img src={orbital_logo} alt="orbital logo" className="h-10 w-auto" />
      </Link>

      {/* Navigation Links */}
      <div className="flex space-x-7">
        {NAVIGATION_LINKS.map((link) => (
          <Link
            key={link.url}
            to={link.url}
            className="hover:underline transition-colors"
          >
            {link.text}
          </Link>
        ))}
      </div>


      <div className="flex items-center gap-4">

        <button
          onClick={toggleTheme}
          className="w-10 h-10 flex items-center justify-center rounded-full border border-foreground/20 hover:bg-accent transition-colors"
          aria-label="Toggle theme"
          title={`Switch to ${theme === "light" ? "dark" : "light"} mode`}
        >
          {theme === "light" ? (

            <svg
              xmlns="http://www.w3.org/2000/svg"
              width="18"
              height="18"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              strokeWidth="2"
              strokeLinecap="round"
              strokeLinejoin="round"
            >
              <path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z" />
            </svg>
          ) : (

            <svg
              xmlns="http://www.w3.org/2000/svg"
              width="18"
              height="18"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              strokeWidth="2"
              strokeLinecap="round"
              strokeLinejoin="round"
            >
              <circle cx="12" cy="12" r="4" />
              <path d="M12 2v2" />
              <path d="M12 20v2" />
              <path d="m4.93 4.93 1.41 1.41" />
              <path d="m17.66 17.66 1.41 1.41" />
              <path d="M2 12h2" />
              <path d="M20 12h2" />
              <path d="m6.34 17.66-1.41 1.41" />
              <path d="m19.07 4.93-1.41 1.41" />
            </svg>
          )}
        </button>


        {isLoggedIn ? (
          <Link
            to="/profile"
            className="border border-foreground/20 rounded-xl px-4 py-2 hover:bg-accent transition-colors"
          >
            Profile
          </Link>
        ) : (
          <Link
            to="/login"
            className="border border-foreground/20 rounded-xl px-4 py-2 hover:bg-accent transition-colors"
          >
            Login
          </Link>
        )}
      </div>
    </nav>
  );
}

export default Nav;
