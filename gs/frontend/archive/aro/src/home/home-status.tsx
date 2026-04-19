import "./home-status.css";
/**
 * @file HomeStatus.tsx
 * @brief Displays the status of the most recent request by a user or a message if there are no requests
 * @todo Implement the status of the most recent request
 */
const HomeStatus = () => {
  return (
    <div className="home-status">
      <h1>You currently do not have any picture requests.</h1>
      <h1>Please create one.</h1>
    </div>
  );
};
export default HomeStatus;
