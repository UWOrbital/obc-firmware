import React, { useState, useEffect } from 'react';
import { mockCommandsList, type ExtendedCommand, type CommandParameter } from '../../../utils/mock-data';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faSpinner } from '@fortawesome/free-solid-svg-icons';
import { Button } from "@/components/ui/button"
import {
  Field,
  FieldDescription,
  FieldGroup,
  FieldLabel,
  FieldLegend,
  FieldSet,
} from "@/components/ui/field"
import { Input } from "@/components/ui/input"
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select"
import CustomAlert from '@/components/Alert';

interface ParameterValues {
  [key: string]: string;
}

const SubmitStatus = {
  None: 'NONE',
  Success: 'SUCCESS',
  InvalidForm: 'INVALID_FORM',
  UnknownError: 'UNKNOWN_ERROR'
} as const;

type SubmitStatus = typeof SubmitStatus[keyof typeof SubmitStatus];

const submitAlerts: Record<SubmitStatus, { destructive: boolean; title: string; description: string; timeout?: number | null }> = {
  [SubmitStatus.None]: { destructive: false, title: "", description: "" },
  [SubmitStatus.Success]: { destructive: false, title: "Success — Command submitted!", description: "", timeout: 7000 },
  [SubmitStatus.InvalidForm]: { destructive: true, title: "Form Invalid. Please fill in all required fields with valid values.", description: "", timeout: null },
  [SubmitStatus.UnknownError]: { destructive: true, title: "An unknown error occurred. Please try again.", description: "", timeout: null },
};

/**
 * @brief SendCommand component for displaying and submitting command parameters
 * @return tsx element of SendCommand component
 */
function SendCommand({ selectedCommandName, setCommand }: { selectedCommandName: string; setCommand: (cmd: string) => void; }) {
  const [matchedCommand, setMatchedCommand] = useState<ExtendedCommand | null>(null);
  const [parameterValues, setParameterValues] = useState<ParameterValues>({});
  const [isSubmitting, setIsSubmitting] = useState<boolean>(false);
  const [currentSubmitStatus, setCurrentSubmitStatus] = useState<SubmitStatus>(SubmitStatus.None);

  // Detect selectedCommandName changes and update states accordingly
  useEffect(() => {
    if (selectedCommandName != "") {
      const command = mockCommandsList.find(cmd => cmd.name === selectedCommandName);
      if (command) {
        setMatchedCommand(command);
        // Initialize parameter values
        const initialValues: ParameterValues = {};
        command.parameters.forEach(param => {
          initialValues[param.name] = '';
        });
        setParameterValues(initialValues);
      } else {
        setMatchedCommand(null);
        setParameterValues({});
      }
    } else {
      setMatchedCommand(null);
      setParameterValues({});
    }
  }, [selectedCommandName]);

  // Handle changes to parameter input fields
  const handleParameterChange = (paramName: string, value: string) => {
    setParameterValues(prev => ({
      ...prev,
      [paramName]: value
    }));
  };

  // TODO: Replace validateParameter with command-specific data validation
  // Validate input based on parameter type
  const validateParameter = (param: CommandParameter, value: string): boolean => {
    if (!value.trim()) {
      return false;
    }

    if (!value.trim()) return true;

    switch (param.type) {
      case 'int': {
        const intValue = parseInt(value);
        if (isNaN(intValue)) return false;
        // Check byte size constraints if specified
        if (param.size) {
          const maxValue = Math.pow(2, param.size * 8) - 1;
          if (intValue < 0 || intValue > maxValue) return false;
        }
        return true;
      }
      case 'float': {
        return !isNaN(parseFloat(value));
      }
      case 'boolean': {
        return value.toLowerCase() === 'true' || value.toLowerCase() === 'false';
      }
      case 'string': {
        return true;
      }
      default: {
        return true;
      }
    }
  };

  // Check if all required parameters are valid by running validateParameter on each parameter
  const isFormValid = (): boolean => {
    if (!matchedCommand) return false;

    return matchedCommand.parameters.every(param => {
      const value = parameterValues[param.name] || '';
      return validateParameter(param, value);
    });
  };

  // Handle command submission
  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();

    if (!matchedCommand || !isFormValid()) {
      setCurrentSubmitStatus(SubmitStatus.InvalidForm);
      return;
    }
    setCurrentSubmitStatus(SubmitStatus.None);
    setIsSubmitting(true);

    try {
      // TODO: adapt data to actual request structure expected by backend
      const submissionData = {
        commandId: matchedCommand.id,
        commandName: matchedCommand.name,
        parameters: parameterValues
      };

      // Simulated API call (replace with actual API call)
      await new Promise(resolve => setTimeout(resolve, 1000));

      // TODO: Setup actual API call
      // const response = await fetch('/api/commands', {
      //   method: 'POST',
      //   headers: { 'Content-Type': 'application/json' },
      //   body: JSON.stringify(submissionData)
      // });

      console.log('Submitting command:', submissionData);

      setCurrentSubmitStatus(SubmitStatus.Success);

      const initialValues: ParameterValues = {};
      matchedCommand.parameters.forEach(param => {
        initialValues[param.name] = '';
      });
      setParameterValues(initialValues);

    } catch (error) {
      console.error('Error submitting command:', error);
      setCurrentSubmitStatus(SubmitStatus.UnknownError);
    } finally {
      setIsSubmitting(false);
    }
  };

  // Render parameter input fields based on type
  const renderParameterInput = (param: CommandParameter) => {
    const value = parameterValues[param.name] || '';
    const isValid = validateParameter(param, value);
    const inputId = `param-${param.name}`;

    const baseInputClasses = `
      w-full px-3 py-2 border rounded-md
      focus:outline-none focus:ring-2 focus:ring-blue-500
      ${!isValid && value ? 'border-red-500 bg-red-50' : 'border-gray-300'}
    `;

    switch (param.type) {
      case 'boolean':
        return (
          <Select
            value={value}
            onValueChange={(val) => handleParameterChange(param.name, val)}
          >
            <SelectTrigger className={`w-[180px] ${!isValid && value ? 'border-red-500 bg-red-50' : 'border-gray-300'}`}>
              <SelectValue placeholder="Select..." />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="true">True</SelectItem>
              <SelectItem value="false">False</SelectItem>
            </SelectContent>
          </Select>
        );

      case 'int':
      case 'float':
        return (
          <Input
            id={inputId}
            type="number"
            value={value}
            onChange={(e) => handleParameterChange(param.name, e.target.value)}
            className={baseInputClasses}
            placeholder={param.type === 'int' ? 'Enter integer' : 'Enter decimal number'}
            step={param.type === 'float' ? 'any' : '1'}
          />
        );

      default:
        return (
          <Input
            id={inputId}
            type="text"
            value={value}
            onChange={(e) => handleParameterChange(param.name, e.target.value)}
            className={baseInputClasses}
            placeholder="Enter text"
          />
        );
    }
  };

  if (!matchedCommand) {
    return
  }

  return (
    <div className="p-4 space-y-6 bg-card w-96 border rounded-md animate-in zoom-in-75 duration-300 slide-in-from-left-10">
        {currentSubmitStatus !== SubmitStatus.None && (
            <CustomAlert destructive={submitAlerts[currentSubmitStatus].destructive} title={submitAlerts[currentSubmitStatus].title} description={submitAlerts[currentSubmitStatus].description} timeout={submitAlerts[currentSubmitStatus].timeout} />
        )}
      <form onSubmit={handleSubmit}>
        <FieldGroup>
          <FieldSet>
            <FieldLegend>{ matchedCommand.name }</FieldLegend>
            <FieldDescription>
              Command ID: { matchedCommand.id } | Format: { matchedCommand.format } | Data Size: { matchedCommand.data_size } bytes | Total Size: { matchedCommand.total_size } bytes
            </FieldDescription>
            <FieldGroup>

              {matchedCommand.parameters.map((param, index) => {
                const value = parameterValues[param.name] || '';
                const isValid = validateParameter(param, value);
                return (
                <Field key={index}>
                  <FieldLabel htmlFor={`param-${param.name}`}>
                    {param.name}
                  </FieldLabel>
                  {renderParameterInput(param)}
                  <div className="transition-all duration-300 overflow-hidden" style={{ maxHeight: !isValid && value ? '3rem' : '0' }}>
                    {!isValid && value && (
                      <p className="text-sm text-red-600 animate-in fade-in-50 duration-150">
                        Invalid {param.type} value
                        {param.size && param.type === 'int' && (
                          ` (must be 0-${Math.pow(2, param.size * 8) - 1})`
                        )}
                      </p>
                    )}
                  </div>
                </Field>
                )
              })}
            </FieldGroup>
          </FieldSet>
          <Field orientation="horizontal">
            <Button type="submit" disabled={!isFormValid() || isSubmitting}>
              Submit
              {isSubmitting && (
                <FontAwesomeIcon icon={faSpinner} className="animate-spin" />
              )}
            </Button>
            <Button variant="outline" type="button" onClick={() => setCommand("")} disabled={isSubmitting}>
              Cancel
            </Button>
          </Field>
        </FieldGroup>
      </form>
    </div>
  );
}

export default SendCommand;
