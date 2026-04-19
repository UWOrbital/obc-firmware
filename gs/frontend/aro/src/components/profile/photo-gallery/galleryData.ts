import spaceImage from "../../../assets/space_dummy_image.png";
import type { Slide } from "./GallerySection";

export function generateSlides(src: string, count = 20, year = 2025, month = 11): Slide[] {
  const slides: Slide[] = [];
  const daysInMonth = new Date(year, month, 0).getDate();

  for (let i = 0; i < count; i++) {
    const day = Math.floor((i * daysInMonth) / count) + 1;
    const hour = 10 + (i % 12);
    const minute = (i * 7) % 60;

    const date = new Date(year, month - 1, day, hour, minute);

    slides.push({
      src,
      width: 320,
      height: 240,
      date: date,
      location: "43.4706° N, -80.5450° W",
    });
  }
  return slides;
}

export const pinnedSlides = generateSlides(spaceImage, 4, 2025, 11);

export type MonthSection = {
  title: string;
  year: number;
  month: number;
  slides: Slide[];
};

export const monthSections: MonthSection[] = [
  { title: "October 2025", year: 2025, month: 10, slides: generateSlides(spaceImage, 19, 2025, 10) },
  { title: "November 2025", year: 2025, month: 11, slides: generateSlides(spaceImage, 19, 2025, 11) },
];

export function sortSlidesByDate(slides: Slide[], order: "asc" | "desc" = "desc"): Slide[] {
  return [...slides].sort((a, b) => {
    const diff = a.date.getTime() - b.date.getTime();
    return order === "asc" ? diff : -diff;
  });
}

export function getSortedMonthSections(order: "asc" | "desc" = "desc"): MonthSection[] {
  const sorted = [...monthSections].sort((a, b) =>
    order === "asc" ? a.year - b.year || a.month - b.month : b.year - a.year || b.month - a.month
  );

  return sorted.map(section => ({
    ...section,
    slides: sortSlidesByDate(section.slides, order),
  }));
}
