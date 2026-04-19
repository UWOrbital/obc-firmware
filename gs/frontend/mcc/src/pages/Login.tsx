/**
 * @brief Login component displaying the login page
 * @return tsx element of Login component
 */
function Login() {
  return (
    <div className="flex flex-col items-center justify-center min-h-screen px-4">
      <div className="w-full max-w-md bg-gray-900/50 backdrop-blur-sm rounded-lg p-8 border border-gray-700/50">
        <h1 className="text-white text-3xl font-bold mb-6 text-center">
          Login
        </h1>
        <form className="space-y-4">
          <div>
            <label htmlFor="username" className="text-gray-300 block mb-2">
              Username
            </label>
            <input
              type="text"
              id="username"
              className="w-full bg-gray-800/50 text-white px-4 py-2 rounded-lg border border-gray-600/50 focus:outline-none focus:border-gray-500"
              placeholder="Enter your username"
            />
          </div>
          <div>
            <label htmlFor="password" className="text-gray-300 block mb-2">
              Password
            </label>
            <input
              type="password"
              id="password"
              className="w-full bg-gray-800/50 text-white px-4 py-2 rounded-lg border border-gray-600/50 focus:outline-none focus:border-gray-500"
              placeholder="Enter your password"
            />
          </div>
          <button
            type="submit"
            className="w-full bg-white text-black py-2 rounded-lg hover:bg-gray-200 transition-colors mt-6"
          >
            Login
          </button>
        </form>
      </div>
    </div>
  );
}

export default Login;
