import { Chart } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  LineElement,
  PointElement,
  LinearScale,
  Title,
  CategoryScale,
  Legend,
  Tooltip,
} from 'chart.js';

ChartJS.register(LineElement, PointElement, LinearScale, Title, CategoryScale, Legend, Tooltip);

import type { TelemetryDataType } from '../../../utils/mockTelemetryData';

type TelemetryChartProps = {
  type: string;
  telemetryData: TelemetryDataType[];
};

const COLORS = [
  'rgba(255, 99, 132, 1)',
  'rgba(54, 162, 235, 1)',
  'rgba(255, 206, 86, 1)',
  'rgba(75, 192, 192, 1)',
  'rgba(153, 102, 255, 1)',
  'rgba(255, 159, 64, 1)',
];

/**
 * @brief Component for displaying selected telemetry data of the as a line chart.
 */
function TelemetryChart({ type, telemetryData }: TelemetryChartProps) {
  const allTimestamps = telemetryData[0]?.datapoints.map(d => d.timestamp) || [];

  const data = {
    labels: allTimestamps,
    datasets: telemetryData.map((entry, idx) => {
      const subtype = entry.type;
      return {
        label: subtype,
        data: entry.datapoints.map(d => d.value),
        borderColor: COLORS[idx % COLORS.length],
        backgroundColor: COLORS[idx % COLORS.length],
        fill: false,
        tension: 0.2,
      };
    }),
  };

  const options = {
    responsive: true,
    plugins: {
      legend: { position: 'top' as const },
      title: { display: true, text: `${type} Telemetry` },
    },
    // TODO: Update axes titles and formatting to match real telemetry data
    scales: {
      x: { title: { display: true, text: 'Timestamp' } },
      y: { title: { display: true, text: 'Value' } },
    },
  };

  return (
    <div className="flex-1 min-w-0">
      <Chart type="line" data={data} options={options} />
    </div>
  );
}

export default TelemetryChart;
