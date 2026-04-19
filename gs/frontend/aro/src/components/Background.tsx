import background from "../assets/galaxy.jpg";

/**
 * @brief Background component displaying the Earth image
 * @return tsx element of Background component
 */
function Background() {
  return (
    <img
      src={background}
      alt="background-image"
      className="fixed inset-0 w-full h-full object-cover -z-10"
    />
  );
}

export default Background;
