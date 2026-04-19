/**
 * @brief Signup component for ARO
 * @return tsx element of Signup component
 */
function Signup() {
    return (
        <div className="flex flex-col items-center justify-center min-h-screen px-4 ">
            <div className="w-full max-w-xl bg-white rounded-lg p-6 pt-6">
                <div className="flex flex-col gap-y-1 mb-7">
                    <h1 className="text-black text-lg font-medium text-center">
                        Sign Up a New Aro Account
                    </h1>
                    <h2 className="text-gray-500 text-center">
                        Enter your email below to login to your account
                    </h2>
                </div>
                <form className="space-y-6">
                    <div>
                        <label htmlFor="email" className="text-black block mb-1">
                            Email
                        </label>
                        <input
                            type="text"
                            id="email"
                            className="w-full text-gray-600 px-4 py-2 rounded-lg border border-gray-600"
                            placeholder="Enter Email"
                        />
                    </div>
                    <div>
                        <label htmlFor="password" className="text-black block mb-1">
                            Password
                        </label>
                        <input
                            type="password"
                            id="password"
                            className="w-full text-gray-600 px-4 py-2 rounded-lg border border-gray-600"
                            placeholder="Enter Password"
                        />
                    </div>
                    <button
                        type="submit"
                        className="w-full bg-black text-white py-2 rounded-lg hover:bg-gray-700 transition-colors cursor-pointer"
                    >
                        Sign Up
                    </button>
                </form>
                <button
                    type="button"
                    className="w-full shadow bg-white text-black py-2 rounded-lg hover:bg-gray-200 transition-colors border border-gray-700/20 mt-2 mb-15 cursor-pointer"
                >
                    Continue with Google
                </button>

            </div>
        </div>
    );
}

export default Signup;