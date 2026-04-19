import { useState } from "react";
import type { ProfileData } from "./profile-data.ts";
import { type SubmitHandler, useForm } from "react-hook-form";
import { updateProfile } from "./profile-api.ts";

const ProfileForm = (props: ProfileData) => {
  const [isEdit, setIsEdit] = useState(false);
  const {
    register,
    handleSubmit,
    formState: { errors },
  } = useForm<ProfileData>();

  // TODO: Submit form to backend
  const onSubmit: SubmitHandler<ProfileData> = (data: ProfileData) => {
    console.log(data);
    setIsEdit(false);
    try {
      updateProfile(data);
    } catch (error) {
      console.error(error);
    }
  };

  // TODO: Add better error handling
  return (
    <>
      <form onSubmit={handleSubmit(onSubmit)}>
        <label>Name:</label>
        <input
          type="text"
          defaultValue={props.name}
          {...register("name", { required: true })}
          readOnly={!isEdit}
        />
        {errors.name && <span>Name is required</span>}

        <label>Email:</label>
        <input
          type="text"
          defaultValue={props.email}
          {...register("email", { required: true })}
          readOnly={!isEdit}
        />
        {errors.email && <span>Email is required</span>}

        <label>Phone:</label>
        <input
          type="text"
          defaultValue={props.phone}
          {...register("phone", { required: true, maxLength: 10 })}
          readOnly={!isEdit}
        />
        {errors.phone && <span>Phone is required</span>}

        <label>Call Sign:</label>
        <input
          type="text"
          defaultValue={props.call_sign}
          {...register("call_sign", {
            required: true,
            pattern: /^[A-Za-z0-9]{5,6}$/,
          })}
          readOnly={!isEdit}
        />
        {errors.call_sign && (
          <span>
            Call sign must be 5-6 characters long and contain only letters and
            numbers
          </span>
        )}

        <button type="submit" hidden={!isEdit}>
          Submit
        </button>
      </form>
      <button hidden={isEdit} onClick={() => setIsEdit(true)}>
        {"Edit"}
      </button>
    </>
  );
};

export default ProfileForm;
