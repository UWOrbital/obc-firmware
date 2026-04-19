/**
 * @brief Verify Component showing ARO OTP verificataion form
 * @return tsx element of Verify component
 */
function Verify() {
    const email = "johndoe@gmail.com";

    return (
        <div className="flex flex-col items-center justify-center min-h-screen px-4 ">
            <div className="w-full max-w-xl bg-white rounded-lg p-6 pt-6">
                <div className="flex flex-col gap-y-1 mb-8">
                    <h1 className="text-black text-lg font-medium text-center">
                        Verify Your Account
                    </h1>
                    <h2 className="text-gray-500 text-center">
                        Please Enter the verification code that we sent to {email} in order to verify your account
                    </h2>
                </div>
                <form className="space-y-6">
                    <div>
                        <label htmlFor="otp-code" className="text-black block mb-1">
                            OTP Verification Code
                        </label>
                        <input
                            type="text"
                            id="opt-code"
                            className="w-full text-gray-600 px-4 py-2 rounded-lg border border-gray-600"
                            placeholder="Enter Verification Code"
                        />
                    </div>
                    <button
                        type="submit"
                        className="w-full bg-black text-white py-2 rounded-lg hover:bg-gray-700 transition-colors cursor-pointer"
                    >
                        Confirm Code
                    </button>
                </form>
                <button
                    type="button"
                    className="w-full shadow bg-white text-black py-2 rounded-lg hover:bg-gray-200 transition-colors border border-gray-700/20 mt-2 mb-15 cursor-pointer"
                >
                    Resend Code
                </button>

            </div>
        </div>
    );
}

export default Verify;