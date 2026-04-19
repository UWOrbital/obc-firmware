import background from "../assets/earth_compressed.webp";
import { useTheme } from "../contexts/ThemeContext";

/**
 * @brief Background component displaying the Earth image
 * @return tsx element of Background component
 */
function Background() {
  const { theme } = useTheme();

  return (
    <>
      <img
        src={background}
        alt="background-image"
        className={`fixed inset-0 h-full w-full object-cover -z-20 transition-opacity duration-300 ${
          theme === "dark" ? "opacity-40" : "opacity-30"
        }`}
      />
      <div
        className={`fixed inset-0 -z-10 transition-colors duration-300 ${
          theme === "dark"
            ? "bg-gradient-to-b from-gray-950/90 via-gray-950/70 to-transparent"
            : "bg-gradient-to-b from-slate-100/80 via-slate-100/50 to-transparent"
        }`}
      />
    </>
  );
}

export default Background;
