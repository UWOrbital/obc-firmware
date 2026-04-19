import React, { type ChangeEvent, useState, useEffect } from "react";
import { useQuery, useQueryClient } from "@tanstack/react-query";

interface MapViewProps {
  handleSubmit: (e: React.FormEvent<HTMLFormElement>) => void;
}

const InputForm: React.FC<MapViewProps> = ({handleSubmit}) => {
  const queryClient = useQueryClient();
  const [localLat, setLocalLat] = useState<number | "">("");
  const [localLng, setLocalLng] = useState<number | "">("");

  const { data: location } = useQuery<{ latitude: number; longitude: number }, Error>({
    queryKey: ["coords"],
    queryFn: async () => {
      const cached = queryClient.getQueryData<{ latitude:number, longitude:number }>(["coords"]);
      if (!cached) throw new Error("No coordinates yet");
      return cached;
    },
    staleTime: Infinity,
    retry: true,
  });

  useEffect(() => {
    if (location) {
      setLocalLat(location.latitude);
      setLocalLng(location.longitude);
    }
  }, [location]);


  const handleLatitudeChange = (event: ChangeEvent<HTMLInputElement>) => {
    const value = event.target.value;
    setLocalLat(value === "" ? "" : parseFloat(value));
  };

  const handleLongitudeChange = (event: ChangeEvent<HTMLInputElement>) => {
    const value = event.target.value;
    setLocalLng(value === "" ? "" : parseFloat(value));
  };

  return (
    <div className="flex flex-col items-center justify-start min-h-screen bg-white text-lg pt-24">
      <div className="w-full max-w-lg p-8 bg-white rounded-lg">
        <h1 className="text-3xl text-center -mt-11 mb-10">New Request Form</h1>
        <form onSubmit={handleSubmit} className="flex flex-col space-y-6 w-full">
          <div className="flex flex-col w-full">
            <label className="text-base font-medium text-gray-700 mb-2">Latitude</label>
            <input
              type="number"
              name="latitude"
              placeholder="Enter latitude"
              value={localLat}
              onChange={handleLatitudeChange}
              required
              className="w-full border border-gray-300 rounded-lg px-4 py-3 text-lg focus:outline-none focus:ring-2 focus:ring-blue-400"
            />
          </div>

          <div className="flex flex-col w-full">
            <label className="text-base font-medium text-gray-700 mb-4">Longitude</label>
            <input
              type="number"
              name="longitude"
              placeholder="Enter longitude"
              value={localLng}
              onChange={handleLongitudeChange}
              required
              className="w-full border border-gray-300 rounded-lg px-4 py-3 text-lg focus:outline-none focus:ring-2 focus:ring-blue-400 mb-5"
            />
          </div>

          <div className="flex flex-col w-full space-y-4 mt-4">
            <button
              type="submit"
              name="action"
              value="Validate"
              className="w-full bg-blue-500 hover:bg-blue-600 text-white font-medium py-3 rounded-lg shadow transition text-lg"
            >
              Validate Coordinates
            </button>
            <button
              type="submit"
              name="action"
              value="Submit"
              className="w-full border border-gray-300 text-black font-medium py-3 rounded-lg shadow-sm hover:bg-gray-50 transition text-lg"
            >
              Submit Request
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};

export default InputForm;
