"use client";

import { useEffect, useState } from "react";

interface GanttChartProps {
  data: { processId: number; startTime: number; endTime: number }[];
}

export default function GanttChart({ data }: GanttChartProps) {
  const [maxTime, setMaxTime] = useState(0);

  useEffect(() => {
    if (data.length > 0) {
      setMaxTime(data[data.length - 1].endTime);
    }
  }, [data]);

  return (
    <div className=" items-center gap-4 p-4 bg-gradient-to-br from-pink-100 to-orange-100 rounded-lg shadow-md w-full max-w-3xl mx-auto">
      <h2 className="text-lg font-semibold text-gray-800">Gantt Chart</h2>
      <div className="relative w-full border border-gray-300 rounded-md overflow-x-auto">
        <div className="flex w-full">
          {data.map((item) => {
            const widthPercentage = ((item.endTime - item.startTime) / maxTime) * 100;
            return (
              <div
                key={item.processId}
                className="border-r border-green-500 text-sm flex flex-col justify-center items-center p-2"
                style={{ width: `${widthPercentage}%`, minWidth: "60px" }}
              >
                <span className="font-medium text-gray-700">P{item.processId}</span>
                <div className="flex justify-between w-full text-xs text-gray-600">
                  <span>{item.startTime}</span>
                  <span>{item.endTime}</span>
                </div>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
}