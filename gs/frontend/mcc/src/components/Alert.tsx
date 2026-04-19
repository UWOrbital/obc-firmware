import { useState, useEffect } from 'react'
import { Alert, AlertDescription, AlertTitle } from './ui/alert'
import { AlertCircleIcon, CheckCircle2Icon } from 'lucide-react'

type CustomAlertProps = {
  destructive?: boolean;
  title?: string;
  description?: string;
  timeout?: number | null;
};

function CustomAlert({ destructive = false, title = "", description = "", timeout = null }: CustomAlertProps) {
  const [isVisible, setIsVisible] = useState(true);

  useEffect(() => {
    if (timeout !== null) {
      const timer = setTimeout(() => {
        setIsVisible(false);
      }, timeout);

      return () => clearTimeout(timer);
    }
  }, [timeout]);

  if (!isVisible) {
    return null;
  }


  return (
    <Alert variant={destructive ? "destructive" : "default"}
    className={`mb-4 ${destructive ? 'bg-red-300/20 text-red-500' : 'bg-green-300/20 text-green-500'} animate-in fade-in slide-in-from-top-5 duration-300`}>
        {destructive ? <AlertCircleIcon className="h-4 w-4" /> : <CheckCircle2Icon className="h-4 w-4" />}
        <AlertTitle>{title}</AlertTitle>
        {description != "" && (
          <AlertDescription>
            {description}
          </AlertDescription>
        )}
    </Alert>
  )
}

export default CustomAlert
