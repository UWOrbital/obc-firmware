import "./new-request-form.css";
import InputForm from "./input-form";
import MapView from "./map-view";
import { useQuery, useQueryClient } from "@tanstack/react-query";
import  React, { useEffect } from "react";

export function isInvalidCoordinate(lat: number, lng: number) {
  if (lat < -90 || lat > 90 || lng < -180 || lng > 180) {
    return true;
  } else {
    return false;
  }
}

export async function getUserLocation(): Promise<{ latitude: number; longitude: number }> {
  return new Promise((resolve, reject) => {
    if (!navigator.geolocation) {
      reject(new Error("Geolocation not supported"));
    }

    navigator.geolocation.getCurrentPosition(
      (position) => {
        resolve({
          latitude: position.coords.latitude,
          longitude: position.coords.longitude,
        });
      },
      (error) => {
        console.error("Geolocation error:", error);
        reject(error);
      }
    );
  });
}

const NewRequestForm = () => {
  useEffect(() => {
    alert('Welcome to the new request form! Enter your coordinates on the left or select them on the map by holding Shift and clicking your desired location.');
  }, []);

  const queryClient = useQueryClient();
  const { data: location, isLoading, isError } = useQuery({
    queryKey: ["coords"],
    queryFn: getUserLocation,
    staleTime: Infinity,
    retry: false,
  });

  const latitude = location?.latitude ?? null;
  const longitude = location?.longitude ?? null;

  const handleSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();

    const formData = new FormData(e.currentTarget);
    const newLatitude = parseFloat(formData.get("latitude") as string);
    const newLongitude = parseFloat(formData.get("longitude") as string);

    const nativeEvent = e.nativeEvent as SubmitEvent;
    const submitter = nativeEvent.submitter as HTMLButtonElement | null;
    const action = submitter?.value;

    if (isNaN(newLatitude) || isNaN(newLongitude)) return;

    if (isInvalidCoordinate(newLatitude, newLongitude)) {
      alert("Please enter valid coordinates!");
      return;
    }

    queryClient.setQueryData(["coords"], {
      latitude: newLatitude,
      longitude: newLongitude,
    });

    if (action === "Validate") {
      alert("Coordinates validated and saved!");
    } else if (action === "Submit") {
      alert(`Request submitted at (${newLatitude}, ${newLongitude})`);
    }
  };

  if (isLoading) {
    return <div className="p-4 text-gray-600">Fetching your location...</div>;
  }

  if (isError) {
    return <div className="p-4 text-red-600">Failed to get your location.</div>;
  }

  return (
    <div className = "form-container flex mt-25">
      <div className="w-1/4 max-h-500 overflow-auto">
        <InputForm
          handleSubmit={handleSubmit}
        />
      </div>
    {latitude !== null && longitude !== null && (
      <div className="flex-1">
        <MapView/>
      </div>
    )}
  </div>
  );
};

export default NewRequestForm;
