/**
 * @brief Component for selecting telemetry subtype(s) available for a given telemetry type
 */
function SubtypeSelector({
  subTypeList,
  selectedSubTypeList,
  setSelectedSubTypeList,
}: {
  subTypeList: string[];
  selectedSubTypeList: string[];
  setSelectedSubTypeList: (selectedSubTypeList: string[]) => void;
}) {
  return (
    <div className="max-w-30 mt-0 flex flex-col gap-y-3">
      {subTypeList.map((st) => (
        <div key={st} className="flex gap-x-2">
          <input
            type="checkbox"
            id={st}
            name={st}
            value={st}
            checked={selectedSubTypeList.includes(st)}
            onChange={(e) => {
              if (e.target.checked) {
                if (!selectedSubTypeList.includes(st)) {
                  setSelectedSubTypeList([...selectedSubTypeList, st]);
                  console.log(selectedSubTypeList);
                }
              } else {
                setSelectedSubTypeList(selectedSubTypeList.filter((item) => item !== st));
              }
            }}
          />
          <label htmlFor={st}>{st}</label>
        </div>
      ))}
    </div>
  );
}

export default SubtypeSelector;
