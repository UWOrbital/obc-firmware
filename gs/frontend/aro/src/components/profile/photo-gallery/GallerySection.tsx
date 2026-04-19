import * as React from "react";
import Lightbox from "yet-another-react-lightbox";
import Zoom from "yet-another-react-lightbox/plugins/zoom";
import Captions from "yet-another-react-lightbox/plugins/captions";
import { Download, Pin } from "lucide-react";
import "yet-another-react-lightbox/styles.css";
import "yet-another-react-lightbox/plugins/captions.css";

function formatTimestamp(date: Date): string {
  const dayNames = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
  const monthNames = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];

  const dayName = dayNames[date.getDay()];
  const monthName = monthNames[date.getMonth()];
  const day = date.getDate();
  const year = date.getFullYear();
  const hours = date.getHours();
  const minutes = date.getMinutes();

  const ampm = hours >= 12 ? "PM" : "AM";
  const displayHour = hours > 12 ? hours - 12 : hours === 0 ? 12 : hours;
  const displayMinute = minutes.toString().padStart(2, '0');

  return `${dayName} - ${monthName} ${day}, ${year} - ${displayHour}:${displayMinute} ${ampm}`;
}

export type Slide = {
  src: string;
  width: number;
  height: number;
  date: Date;
  location?: string;
  title?: string;
  description?: string;
};

interface GallerySectionProps {
  title: string;
  photos: Slide[];
  targetRowHeight?: number;
  spacing?: number;
}

export default function GallerySection({
  title,
  photos,
  spacing = 12,
}: GallerySectionProps) {
  const [index, setIndex] = React.useState(-1);

  const handleDownload = React.useCallback(() => {
    if (index < 0 || !photos[index]) return;
    const link = document.createElement("a");
    link.href = photos[index].src;
    link.download = `photo-${formatTimestamp(photos[index].date)}.jpg`;
    link.click();
  }, [index, photos]);

  const handlePin = React.useCallback(() => {
    // TODO: Implement pin functionality
  }, [index]);

  const slidesWithCaptions = React.useMemo(() =>
    photos.map(photo => ({
      ...photo,
      title: formatTimestamp(photo.date),
      description: photo.location || "",
    })),
    [photos]
  );

  return (
    <section className="pt-4 pb-6">
      <div className="flex flex-col items-center w-full">
        <div className="mb-4 w-full flex justify-center">
          <h3 className="text-2xl font-semibold text-gray-900 text-center">{title}</h3>
        </div>
        <div
          className="flex flex-wrap justify-center"
          style={{ gap: typeof spacing === "number" ? `${spacing}px` : "12px" }}
        >
          {photos.map((p, i) => (
            <button
              key={`${p.src}-${i}`}
              type="button"
              onClick={() => setIndex(i)}
              className="w-[200px] h-[150px] overflow-hidden rounded-lg bg-gray-100 shadow-sm hover:shadow-lg transition-all duration-300 cursor-pointer focus:outline-none focus:ring-2 focus:ring-gray-300"
              style={{ flex: "0 0 auto" }}
              aria-label={`Open image ${i + 1}`}
            >
              <img
                src={p.src}
                className="w-full h-full object-cover transition-transform duration-300 hover:scale-110"
                width={200}
                height={150}
                loading="lazy"
              />
            </button>
          ))}
        </div>
      </div>

      <Lightbox
        open={index >= 0}
        index={index}
        close={() => setIndex(-1)}
        slides={slidesWithCaptions}
        plugins={[Zoom, Captions]}
        zoom={{
          maxZoomPixelRatio: 5,
          zoomInMultiplier: 2,
          doubleTapDelay: 300,
          doubleClickDelay: 300,
          doubleClickMaxStops: 2,
          keyboardMoveDistance: 50,
          wheelZoomDistanceFactor: 100,
          pinchZoomDistanceFactor: 100,
          scrollToZoom: true,
        }}
        captions={{
          showToggle: true,
          descriptionTextAlign: "start",
        }}
        toolbar={{
          buttons: [
            <button
              key="pin"
              type="button"
              onClick={handlePin}
              className="yarl__button"
              aria-label="Pin photo"
            >
              <Pin className="w-5 h-5" />
            </button>,
            <button
              key="download"
              type="button"
              onClick={handleDownload}
              className="yarl__button"
              aria-label="Download photo"
            >
              <Download className="w-5 h-5" />
            </button>,
            "close",
          ],
        }}
      />
    </section>
  );
}
