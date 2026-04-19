/**
 * @brief PageNotFound/404 error fallback component for ARO
 * @return tsx element of PageNotFound component
 */

import { Link } from "react-router-dom";
import Astronaut from "../assets/astronaut.svg"

function PageNotFound() {
  return (
    <div className="flex items-center justify-center min-h-screen pb-48">
        <div className="w-64 h-64 relative">
            <img src={Astronaut} />
        </div>
        <div className="space-y-1">
            <h1 className="text-4xl">lost in space . . .</h1>
            <p className="text-xl">The page you are looking for doesn't exist or has been moved.</p>
            <p>Return to <Link className="underline" to="/">homepage</Link></p>
        </div>
    </div>
  )
}

export default PageNotFound;