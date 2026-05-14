import {
  telemetrySubtypes,
  telemetryTypes,
} from "../../../utils/mockTelemetryData";

/**
 *
 * @brief Component for selecting telemetry type, and updates subtype list accordingly
 *
 */
function TypeSelector({
  type,
  setType,
  setSubType,
  setSelectedSubTypeList,
}: {
  type: string;
  setType: (type: string) => void;
  setSubType: (subType: string[]) => void;
  setSelectedSubTypeList: (selectedSubTypeList: string[]) => void;
}) {
  return (
    <div className="p-2 bg-card w-fit rounded-lg">
      <div className="flex gap-x-2 text-lg">
        {telemetryTypes.map((t, i) => (
          <>
            <button
              key={t}
              className={
                type === t ? "bg-card p-2 py-0 rounded-lg" : "p-2 py-0"
              }
              onClick={() => {
                setSelectedSubTypeList(telemetrySubtypes[t]);
                console.log(telemetrySubtypes[t]);
                setType(t);
                setSubType(telemetrySubtypes[t]);

              }}
            >
              {t}
            </button>
            {i !== telemetryTypes.length - 1 && (
              <span className="py-1"> | </span>
            )}
          </>
        ))}
      </div>
    </div>
  );
}

export default TypeSelector;
