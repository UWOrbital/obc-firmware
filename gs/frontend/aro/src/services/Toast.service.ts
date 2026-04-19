import { toast, type ToastOptions } from "react-toastify";

const baseToastConfig: ToastOptions = {
  position: "top-right",
  autoClose: 4900,
  hideProgressBar: false,
  closeOnClick: true,
  pauseOnHover: true,
  draggable: true,
  progress: undefined,
  theme: "dark",
};

/**
 * Displays a success toast notification.
 * Uses Tailwind classes directly for styling.
 */
export const success = (message: string): void => {
  toast.success(message, {
    ...baseToastConfig,
    className:
      "bg-black/75 border border-green-500 text-green-100 rounded-md shadow-md text-sm font-medium",
    progressClassName: "bg-green-500",
  });
};

/**
 * Displays an error toast notification.
 * Uses Tailwind classes directly for styling.
 */
export const error = (message: string): void => {
  toast.error(message, {
    ...baseToastConfig,
    className:
      "bg-black/75 border border-red-500 text-red-100 rounded-md shadow-md text-sm font-medium",
    progressClassName: "bg-red-500",
  });
};

const toastService = {
  success,
  error,
};

export default toastService;
