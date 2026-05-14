type ThesholdSelectorProps = {
  threshold: string | number;
  setThreshold: (value: string | number) => void;
};

/*
* @brief Component for selecting threshold value
*/
function ThesholdSelector({ threshold, setThreshold }: ThesholdSelectorProps) {
  return (
    <div className='bg-card p-2 gap-x-2 w-fit rounded-lg text-lg flex items-center'>
      <span>Threshold:</span>
      <input
        type="number"
        value={threshold}
        onChange={e => setThreshold(e.target.value)}
        className="border border-card rounded px-2 py-1 text-foreground focus:outline-none focus:ring-2 focus:ring-primary/30"
        min={0}
        step={0.01}
      />
    </div>
  );
}

export default ThesholdSelector
