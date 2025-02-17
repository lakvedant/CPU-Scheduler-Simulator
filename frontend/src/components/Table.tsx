'use client'
import React, { useState, useEffect } from "react";
import {
  Table,
  TableBody,
  TableCaption,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from "@/components/ui/table";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Skeleton } from "@/components/ui/skeleton";
import { Badge } from "@/components/ui/badge";

interface Process {
  id: number;
  burstTime: number;
  arrivalTime: number;
  priority: number;
}

const ProcessTable = () => {
  const [processes, setProcesses] = useState<Process[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const fetchProcesses = async () => {
      try {
        const response = await fetch(`/api/processes`);
        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        setProcesses(data.sort((a: Process, b: Process) => a.arrivalTime - b.arrivalTime));
      } catch (error) {
        setError("Error fetching processes. Please try again later.");
        console.error("Error fetching processes:", error);
      } finally {
        setLoading(false);
      }
    };

    fetchProcesses();
  }, []);

  const getPriorityColor = (priority: number) => {
    if (priority <= 2) return "destructive";
    if (priority <= 4) return "secondary";
    return "default";
  };

  return (
    <Card className="w-full max-w-4xl mx-auto">
      <CardHeader className="bg-gray-50">
        <CardTitle>Process Scheduling Table</CardTitle>
      </CardHeader>
      <CardContent>
        {loading ? (
          <div className="space-y-2">
            {[...Array(5)].map((_, index) => (
              <Skeleton key={index} className="h-8 w-full" />
            ))}
          </div>
        ) : error ? (
          <div className="text-red-500 font-medium py-4 text-center">{error}</div>
        ) : (
          <Table>
            <TableCaption>Processes sorted by arrival time</TableCaption>
            <TableHeader>
              <TableRow>
                <TableHead>Process ID</TableHead>
                <TableHead>Burst Time</TableHead>
                <TableHead>Arrival Time</TableHead>
                <TableHead className="text-right">Priority</TableHead>
              </TableRow>
            </TableHeader>
            <TableBody>
              {processes.map((process) => (
                <TableRow key={process.id}>
                  <TableCell className="font-medium">{process.id}</TableCell>
                  <TableCell>{process.burstTime} ms</TableCell>
                  <TableCell>{process.arrivalTime} ms</TableCell>
                  <TableCell className="text-right">
                    <Badge variant={getPriorityColor(process.priority)}>
                      {process.priority}
                    </Badge>
                  </TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        )}
      </CardContent>
    </Card>
  );
};

export default ProcessTable;