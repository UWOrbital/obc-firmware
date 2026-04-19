import { useEffect, useRef, useState, type FormEventHandler } from "react";
import "./profile-form.css"

interface FormInputs {
  fname: string
  lname: string
  email: string
  phone: string
  callsign: string
}
const emptyInputs: FormInputs = {
  fname: "",
  lname: "",
  email: "",
  phone: "",
  callsign: ""
}
const mockInputsData: FormInputs = {
  fname: "first",
  lname: "last",
  email: "hi@hi.com",
  phone: "8880001234",
  callsign: "V0V0V0"
}


export default function ProfileForm() {
  const [fileName, setFileName] = useState(""); // state of file name of file uploaded
  const [inputDefaults, setInputDefaults] = useState<FormInputs>(emptyInputs); // defaults for (form) input
  const fileInputRef = useRef<HTMLInputElement>(null); // ref to more easily access file input element

  useEffect(() => {
    // fetch initial data on mount
    const initialDefaultData = fetchInitialData();
    setInputDefaults(initialDefaultData);
  }, []);

  const fetchInitialData = () => {
    // TODO, fetch user data here
    return mockInputsData;
  }

  const handleSubmit: FormEventHandler<HTMLFormElement> = (e) => {
    e.preventDefault();
    // set defaults to new values
    const submitFormInputs: FormInputs = {
      fname: e.currentTarget.fname.value,
      lname: e.currentTarget.lname.value,
      email: e.currentTarget.email.value,
      phone: e.currentTarget.phone.value,
      callsign: e.currentTarget.callsign.value
    }
    console.log(submitFormInputs);
    // TODO: do something with the submitted data


    setInputDefaults(submitFormInputs);
  }

  const handleUpload = () => {
    const fileInput = fileInputRef.current; // get file input element
      if (!fileInput || !fileInput.files || fileInput.files.length === 0) {
        console.log("Please select a file to upload");
        return;
      }

      // Extract the first file from the file input
      const file = fileInput.files[0];

      // custom file validation, set as needed these are just some examples
      if (file.size >= 5 * 1024 * 1024) {
        console.log("Max 5MB image upload");
        return;
      } else if (!file.type.startsWith("image/")) {
        console.log("Invalid image format");
        return;
      }
      console.log(`${file.name} can be uploaded! (success message)`);
      // TODO: do something with image if we decide to use pfps
  }

  return (
    <div className="flex w-auto h-[calc(100vh-7rem)] max-h-full max-w-screen justify-center items-center-safe mt-24 mx-4 p-4 overflow-auto">
      <div className="flex flex-wrap items-center justify-evenly max-w-full h-fit rounded-2xl bg-white gap-4 p-6">

        {/* profile picture left bar */}
        <div className="flex flex-col w-60 max-w-full items-center h-fit max-h-full gap-2">

          {/* profile image container */}
          <div className="flex w-full max-w-full aspect-square items-center justify-center rounded-lg bg-indigo-950">
            <img src="/vite.svg" alt="profile image" className="size-full object-cover"/>
          </div>

          {/* pfp edit buttons */}
          <label htmlFor="pfp-upload" className="w-full max-w-full px-2 py-1 transition-colors hover:bg-gray-200 text-gray-900 border-black border rounded-lg text-center cursor-pointer shadow">
            Edit profile image
          </label>
          <input id="pfp-upload" type="file" hidden ref={fileInputRef} accept="image/*" onChange={(e) => setFileName(e.currentTarget.files?.item(0)?.name ?? "")}/>
          {fileName && /* only load file name and upload button once file is received */
            <div className="flex flex-wrap items-center justify-center h-fit w-full gap-2">
              <p className="text-gray-900 max-w-full wrap-break-word">{fileName}</p>
              <button onClick={handleUpload} className="size-fit px-2 py-1 rounded-lg transition-colors bg-gray-900 hover:bg-gray-700 shadow">Upload</button>
            </div>
          }
        </div>

        {/* edit form right bar */}
        <form id="edit-form" onSubmit={handleSubmit} className="flex flex-col items-end w-xl h-fit p-1 gap-4 overflow-x-auto">
          <label className="w-full text-2xl font-medium text-black wrap-break-word">Edit information</label>

          {/* form text fields */}
          <div className="flex flex-col w-full h-auto gap-2">

            {/* field 1 & 2: first & last name */}
            <div className="flex flex-wrap w-full gap-x-4 gap-y-2">
              <div className="field-label-container grow shrink">
                <label htmlFor="fname-field">First name</label>
                <input id="fname-field" name="fname" placeholder="First name"
                className="form-field" defaultValue={inputDefaults.fname}/>
              </div>
              <div className="field-label-container grow shrink">
                <label htmlFor="lname-field">Last name</label>
                <input id="lname-field" name="lname" placeholder="Last name"
                className="form-field" defaultValue={inputDefaults.lname}/>
              </div>
            </div>

            {/* field 3: email */}
            <div className="field-label-container">
              <label htmlFor="email-field">Email</label>
              <input id="email-field" name="email" type="email" placeholder="Email"
              className="form-field" defaultValue={inputDefaults.email}/>
            </div>

            {/* field 4: phone */}
            <div className="field-label-container">
              <label htmlFor="phone-field">Phone</label>
              <input id="phone-field" name="phone" type="tel" placeholder="Phone"
              className="form-field" defaultValue={inputDefaults.phone}/>
            </div>

            {/* field 5: callsign */}
            <div className="field-label-container">
              <label htmlFor="callsign-field">Callsign</label>
              <input id="callsign-field" name="callsign" placeholder="Callsign"
              className="form-field" defaultValue={inputDefaults.callsign}/>
            </div>

          </div>

          {/* form buttons */}
          <div className="flex flex-wrap size-fit gap-x-3 gap-y-1">
            <button type="submit" className="form-buttons transition-colors bg-gray-900 hover:bg-gray-700 shadow">
              Submit
            </button>
            {/*<button type="reset" className="form-buttons transition-colors hover:bg-gray-200 border border-solid border-gray-900 text-gray-900 shadow">
              Reset
            </button>*/}
          </div>
        </form>
      </div>
    </div>
  );
}
