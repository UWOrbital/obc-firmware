import { clsx, type ClassValue } from "clsx"
import { twMerge } from "tailwind-merge"

// Helper function preventing Tailwind utility conflicts

export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}
