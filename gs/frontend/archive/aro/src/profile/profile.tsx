import { useQuery } from "@tanstack/react-query";
import { getProfile } from "./profile-api.ts";
import ProfileForm from "./profile-form.tsx";

const Profile = () => {
  const { isPending, error, data } = useQuery({
    queryKey: ["profile"],
    queryFn: getProfile,
  });
  if (isPending) return <div>Loading...</div>;
  if (error) return <div>{error.message}</div>;
  if (!data) return <div>No data</div>;

  return <ProfileForm {...data} />;
};

export default Profile;
