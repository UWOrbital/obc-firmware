import logo from "../assets/aro_logo.png";
import { Button } from "./ui/button";
import { Link } from "react-router-dom";

/**
 * @brief Hero component displaying the ARO hero page
 * @return tsx element of Logo component
*/

function Home() {
    return (
        <div className="pt-8 w-full flex flex-col items-center">
            <img
             src={logo}
             alt="aro-logo"
             className="w-3xs md:w-xs lg:w-xs"></img>
            <h1 className="font-[Jaldi] font-bold -mt-5 md:-mt-9 lg:-mt-9 text-5xl md:text-6xl lg:text-8xl">Amateur Radio Operator</h1>
            <p className="font-[Jaldi] font-light mt-4 text-2xl md:text-2xl lg:text-3xl">Please sign up/login to make a request.</p>
            <div className="flex gap-20 mt-10">
                <Button size="lg" variant="outline" className="bg-transparent font-[Jaldi] font-bold" asChild>
                    <Link to="/sign-up">Sign Up</Link>
                </Button>
                <Button size="lg" variant="outline" className="bg-transparent font-[Jaldi] font-bold" asChild>
                    <Link to="/login">Login</Link>
                </Button>
            </div>
        </div>
    )
}

export default Home
