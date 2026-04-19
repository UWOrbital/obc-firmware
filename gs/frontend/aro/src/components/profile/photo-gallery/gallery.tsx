import * as React from "react";
import GallerySection from "./GallerySection";
import { pinnedSlides, getSortedMonthSections, monthSections } from "./galleryData";
import "./gallery.css";

export default function Gallery() {
  const [order, setOrder] = React.useState<"asc" | "desc">("desc");

  {/* TODO: Fetch photos from DB rather than using dummy data */}
  const groupedByYear = React.useMemo(() => {
    const groups: Record<number, typeof monthSections> = {};
    monthSections.forEach((m) => {
      groups[m.year] = groups[m.year] ? [...groups[m.year], m] : [m];
    });
    return groups;
  }, []);

  return (
    <div className="w-full max-w-screen-xl mx-auto px-4 mt-24 mb-24">
      <div className="bg-white rounded-2xl shadow-lg border border-gray-100 overflow-hidden pt-10 pb-6 px-10 relative flex flex-col" style={{ maxHeight: "calc(100vh - 8rem)" }}>
        <div className="mb-1 flex items-center justify-center shrink-0">
          <div className="inline-flex rounded-lg border border-gray-300 bg-white shadow-sm p-1">
            <button
              type="button"
              onClick={() => setOrder("asc")}
              className={`px-3 py-1 text-sm rounded-md transition-colors ${
                order === "asc"
                  ? "bg-gray-900 text-white"
                  : "text-gray-700 hover:bg-gray-100"
              }`}
            >
              Ascending
            </button>
            <button
              type="button"
              onClick={() => setOrder("desc")}
              className={`ml-1 px-3 py-1 text-sm rounded-md transition-colors ${
                order === "desc"
                  ? "bg-gray-900 text-white"
                  : "text-gray-700 hover:bg-gray-100"
              }`}
            >
              Descending
            </button>
          </div>
        </div>

        <div
          className="overflow-y-auto mt-2 gallery-scroll flex flex-col items-center"
          style={{ maxHeight: "calc(100vh - 11rem)" }}
        >
          {/* TODO: Fetch pinned photos from DB rather than using dummy data */}
                    <GallerySection
            title={"Pinned"}
            photos={pinnedSlides}
          />

          {/* TODO: Fetch photos from DB rather than using dummy data */}
          {Object.keys(groupedByYear)
            .sort((a, b) => (order === "asc" ? Number(a) - Number(b) : Number(b) - Number(a)))
            .map((yearKey) => {
              const year = Number(yearKey);
              const sortedMonths = getSortedMonthSections(order).filter((m) => m.year === year);
              return (
                <div key={year} className="mt-10 w-full">
                  <h2 className="text-5xl font-bold text-gray-900 mb-2 text-center">{year}</h2>
                  {sortedMonths.map((m) => (
                    <GallerySection
                      key={m.title}
                      title={m.title}
                      photos={m.slides}
                    />
                  ))}
                </div>
              );
            })}
        </div>
      </div>
    </div>
  );
}
