"use client";

import { SetStateAction, useEffect, useState } from "react";

interface GanttChartProps {
  data: { processId: number; startTime: number; endTime: number }[];
}

export default function GanttChart({ data }: GanttChartProps) {
  const [maxTime, setMaxTime] = useState(0);
  const [processBlocks, setProcessBlocks] = useState<Array<{
    processId: number | null;
    startTime: number;
    endTime: number;
    isIdle: boolean;
  }>>([]);

  useEffect(() => {
    if (data.length > 0) {
      // Sort data by start time
      const sortedData = [...data].sort((a, b) => a.startTime - b.startTime);
      const lastEndTime = Math.max(...data.map(item => item.endTime));
      setMaxTime(lastEndTime);

      // Create blocks including idle time
      const blocks: SetStateAction<{ processId: number | null; startTime: number; endTime: number; isIdle: boolean; }[]> = [];
      let currentTime = 0;

      sortedData.forEach((item) => {
        // Add idle block if there's a gap
        if (item.startTime > currentTime) {
          blocks.push({
            processId: null,
            startTime: currentTime,
            endTime: item.startTime,
            isIdle: true
          });
        }
        
        // Add process block
        blocks.push({
          processId: item.processId,
          startTime: item.startTime,
          endTime: item.endTime,
          isIdle: false
        });
        
        currentTime = item.endTime;
      });

      setProcessBlocks(blocks);
    }
  }, [data]);

  return (
    <div className="items-center gap-4 p-4 bg-gradient-to-br from-pink-100 to-orange-100 rounded-lg shadow-md w-full max-w-3xl mx-auto">
      <h2 className="text-lg font-semibold text-gray-800">Gantt Chart</h2>
      <div className="relative w-full border border-gray-300 rounded-md overflow-x-auto">
        <div className="flex w-full">
          {processBlocks.map((block, index) => {
            const widthPercentage = ((block.endTime - block.startTime) / maxTime) * 100;
            return (
              <div
                key={`${block.processId || 'idle'}-${index}`}
                className={`${
                  block.isIdle 
                    ? 'bg-gray-200 border-gray-300' 
                    : 'border-green-500'
                } border-r text-sm flex flex-col justify-center items-center p-2`}
                style={{ width: `${widthPercentage}%`, minWidth: "60px" }}
              >
                <span className="font-medium text-gray-700">
                  {block.isIdle ? 'Idle' : `P${block.processId}`}
                </span>
                <div className="flex justify-between w-full text-xs text-gray-600">
                  <span>{block.startTime}</span>
                  <span>{block.endTime}</span>
                </div>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
}