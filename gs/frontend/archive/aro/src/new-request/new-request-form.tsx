import "./new-request-form.css";
import { type ChangeEvent, useState } from "react";
import React from "react";

const NewRequestForm = () => {
  const [latitude, setLatitude] = useState(0);
  const [longitude, setLongitude] = useState(0);

  navigator.geolocation.getCurrentPosition((position) => {
    setLatitude(position.coords.latitude);
    setLongitude(position.coords.longitude);
    // TODO: Show a map centered at latitude / longitude.
  });

  const handleSubmit = async (event: React.FormEvent) => {
    event.preventDefault();

    try {
      const submission = {
        latitude,
        longitude,

      };

      const response = await fetch('http://localhost:5000/aro-request', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          // NOTE TO REVIEWERS:
          // Add authentication headers when implemented
          // 'Authorization': `Bearer ${token}`
        },
        body: JSON.stringify(submission),
      });

      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const result = await response.json();
      console.log('Request submitted successfully:', result);

      alert("Request submitted successfully! You can view it in your requests list.");



    } catch (error) {
      console.error('Error submitting request:', error);
      alert("Error submitting request. Please try again.");
    }
  };

  const handleLatitudeChange = (event: ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(event.target.value);
    setLatitude(value);
  };

  const handleLongitudeChange = (event: ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(event.target.value);
    setLongitude(value);
  };

  // TODO: Add better error handling and switch to using react-hook-form
  return (
    <form className="input-form" onSubmit={handleSubmit} id="main-form">
      <label>Latitude</label>
      <input
        required
        type="number"
        placeholder="Enter your coordinates"
        value={latitude}
        onChange={handleLatitudeChange}
      />
      <label>Longitude</label>
      <input
        required
        type="number"
        placeholder="Enter your coordinates"
        value={longitude}
        onChange={handleLongitudeChange}
      />
      <input type="submit" />
    </form>
  );
};

export default NewRequestForm;
