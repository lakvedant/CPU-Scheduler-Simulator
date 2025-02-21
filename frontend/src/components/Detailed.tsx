import React from 'react';
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table";

interface Process {
  id: number;
  arrivalTime: number;
  burstTime: number;
  priority: number;
  completionTime?: number;
  turnaroundTime?: number;
  waitingTime?: number;
  responseTime?: number;
}

interface AlgorithmResult {
  name: string;
  avgTurnaroundTime: number;
  avgWaitingTime: number;
  avgResponseTime: number;
  throughput: number;
  cpuUtilization: number;
  ganttChart: { processId: number; startTime: number; endTime: number }[];
  processes: Process[];
}

interface DetailedMetricsTableProps {
  results: AlgorithmResult[];
}

const DetailedMetricsTable: React.FC<DetailedMetricsTableProps> = ({ results }) => {
  if (!results || results.length === 0) {
    return <div className="text-center text-gray-500">No results available</div>;
  }

  return (
    <div className="space-y-6">
      {results.map((result, algorithmIndex) => {
        // Sort processes by ID
        const sortedProcesses = [...result.processes].sort((a, b) => a.id - b.id);
        
        return (
          <Card key={`algorithm-${algorithmIndex}`} className="overflow-hidden">
            <CardHeader className="bg-gradient-to-r from-indigo-50 to-blue-50">
              <CardTitle className="text-xl text-indigo-800">{result.name}</CardTitle>
            </CardHeader>
            <CardContent className="pt-6">
              <div className="overflow-x-auto">
                <Table>
                  <TableHeader>
                    <TableRow className="bg-gray-50">
                      <TableHead className="text-left">Process ID</TableHead>
                      <TableHead className="text-right">Arrival Time</TableHead>
                      <TableHead className="text-right">Burst Time</TableHead>
                      <TableHead className="text-right">Priority</TableHead>
                      <TableHead className="text-right">Completion Time</TableHead>
                      <TableHead className="text-right">Turnaround Time</TableHead>
                      <TableHead className="text-right">Waiting Time</TableHead>
                      <TableHead className="text-right">Response Time</TableHead>
                    </TableRow>
                  </TableHeader>
                  <TableBody>
                    {sortedProcesses.map((process, processIndex) => {
                      // Calculate turnaround time if not provided
                      const turnaroundTime = process.turnaroundTime ?? 
                        (process.completionTime !== undefined ? 
                          process.completionTime - process.arrivalTime : 
                          undefined);
                      
                      // Calculate waiting time if not provided
                      const waitingTime = process.waitingTime ?? 
                        (turnaroundTime !== undefined ? 
                          turnaroundTime - process.burstTime : 
                          undefined);

                      return (
                        <TableRow 
                          key={`${result.name}-${process.id}-${processIndex}`}
                          className="hover:bg-gray-50"
                        >
                          <TableCell className="font-medium">P{process.id}</TableCell>
                          <TableCell className="text-right">{process.arrivalTime}</TableCell>
                          <TableCell className="text-right">{process.burstTime}</TableCell>
                          <TableCell className="text-right">{process.priority}</TableCell>
                          <TableCell className="text-right">
                            {process.completionTime?.toFixed(0) ?? '-'}
                          </TableCell>
                          <TableCell className="text-right">
                            {turnaroundTime?.toFixed(0) ?? '-'}
                          </TableCell>
                          <TableCell className="text-right">
                            {waitingTime?.toFixed(0) ?? '-'}
                          </TableCell>
                          <TableCell className="text-right">
                            {process.responseTime?.toFixed(0) ?? '-'}
                          </TableCell>
                        </TableRow>
                      );
                    })}
                  </TableBody>
                </Table>
              </div>

              <div className="mt-6 grid grid-cols-2 md:grid-cols-4 gap-4">
                <div className="p-3 rounded-lg bg-blue-50 border border-blue-100">
                  <p className="text-sm font-medium text-blue-800">Avg Turnaround Time</p>
                  <p className="text-lg font-bold text-blue-900">{result.avgTurnaroundTime.toFixed(2)}</p>
                </div>
                <div className="p-3 rounded-lg bg-green-50 border border-green-100">
                  <p className="text-sm font-medium text-green-800">Avg Waiting Time</p>
                  <p className="text-lg font-bold text-green-900">{result.avgWaitingTime.toFixed(2)}</p>
                </div>
                <div className="p-3 rounded-lg bg-purple-50 border border-purple-100">
                  <p className="text-sm font-medium text-purple-800">Avg Response Time</p>
                  <p className="text-lg font-bold text-purple-900">{result.avgResponseTime.toFixed(2)}</p>
                </div>
                <div className="p-3 rounded-lg bg-indigo-50 border border-indigo-100">
                  <p className="text-sm font-medium text-indigo-800">Throughput</p>
                  <p className="text-lg font-bold text-indigo-900">{result.throughput.toFixed(2)}</p>
                </div>
              </div>
            </CardContent>
          </Card>
        );
      })}
    </div>
  );
};

export default DetailedMetricsTable;