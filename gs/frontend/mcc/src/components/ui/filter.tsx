import { useState, useRef, useEffect } from "react";

type FilterOption = "Filter" | "Stars" | "Forks" | "Language" | "Last Updated";

const options: FilterOption[] = ["Filter", "Stars", "Forks", "Language", "Last Updated"];

export default function FilterDropdown() {
  const [isOpen, setIsOpen] = useState<boolean>(false);
  const [selected, setSelected] = useState<FilterOption | null>(null);
  const ref = useRef<HTMLDivElement>(null);

  useEffect(() => {
    function handleClickOutside(e: MouseEvent) {
      if (ref.current && !ref.current.contains(e.target as Node)) {
        setIsOpen(false);
      }
    }
    document.addEventListener("mousedown", handleClickOutside);
    return () => document.removeEventListener("mousedown", handleClickOutside);
  }, []);

  return (
    <div className="relative w-36" ref={ref}>

      <button
        onClick={() => setIsOpen(prev => !prev)}
        className="w-full border-2 border-[#898989] rounded-2xl px-3 text-sm italic font-bold text-sm flex items-center justify-between"
      >
        <span>{selected ?? "Filter"}</span>
        <svg
          className={`w-4 h-4 transition-transform ${isOpen ? "rotate-180" : ""}`}
          fill="none" stroke="currentColor" viewBox="0 0 24 24"
        >
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 9l-7 7-7-7" />
        </svg>
      </button>

      {isOpen && (
        <ul className="absolute z-10 mt-1 w-full border-2 border-[#898989] rounded-xl shadow-md overflow-hidden">
          {options.map(option => (
            <li
              key={option}
              onClick={() => {
                setSelected(option);
                setIsOpen(false);
              }}
              className={`px-3 py-2 text-sm cursor-pointer hover:bg-gray-300 hover:text-black
                ${selected === option ? "font-bold text-black bg-[#898989]" : "font-normal"}`}
            >
              {option}
            </li>
          ))}
        </ul>
      )}

    </div>
  );
}