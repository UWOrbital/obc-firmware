import { MapContainer, TileLayer, useMap, Marker, Popup, useMapEvents } from 'react-leaflet';
import 'leaflet/dist/leaflet.css';
import { useQuery, useQueryClient} from "@tanstack/react-query";
import React from "react";

function RecenterMap({ lat, lng }: { lat: number; lng: number }) {
  const map = useMap();
  React.useEffect(() => {
    map.setView([lat, lng], map.getZoom());
  }, [lat, lng, map]);
  return null;
}

function LocationSelector() {
  const queryClient = useQueryClient();

  useMapEvents({
    click(e) {
      const event = e.originalEvent as MouseEvent;
      if (event.shiftKey) {
        queryClient.setQueryData<{ latitude: number; longitude: number }>(["coords"], {
          latitude: e.latlng.lat,
          longitude: e.latlng.lng,
        });
      }
    },
  });

  return null;
}

const MapView: React.FC = () => {
  const queryClient = useQueryClient();

  const { data } = useQuery<{ latitude: number; longitude: number }>({
    queryKey: ["coords"],
    queryFn: async () => {
      const cached = queryClient.getQueryData<{ latitude: number; longitude: number }>(["coords"]);
      if (!cached) throw new Error("No coordinates yet");
      return cached;
    },
    staleTime: Infinity,
  });

  if (!data) return null;

  return (
    <MapContainer
      center={[data.latitude, data.longitude]}
      zoom={7}
      scrollWheelZoom={false}
      style={{ height: "100%", width: "100%" }}
      doubleClickZoom={false}
      boxZoom={false}
      keyboard={false}
    >
      <TileLayer
        attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
        url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
      />
      <Marker position={[data.latitude, data.longitude]}>
        <Popup>The selected request position</Popup>
      </Marker>
      <LocationSelector />
      <RecenterMap lat={data.latitude} lng={data.longitude} />
    </MapContainer>
  );
};

export default MapView;
