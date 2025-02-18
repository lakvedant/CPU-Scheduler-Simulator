'use client'
import React, { useState } from 'react';
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { ChevronUp, ChevronDown, ArrowUpDown } from 'lucide-react';
import {
  Table,
  TableBody,
  TableCaption,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from "@/components/ui/table";
import { Skeleton } from "@/components/ui/skeleton";

// Types
interface Process {
  id: number;
  burstTime: number;
  arrivalTime: number;
  priority: number;
}

interface SortConfig {
  key: keyof Process;
  direction: 'asc' | 'desc';
}

// ProcessInputForm Component
const ProcessInputForm = ({ onSubmit }: { onSubmit: (count: number) => void }) => {
  const [count, setCount] = useState('');

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    const numCount = parseInt(count);
    if (numCount > 0) {
      onSubmit(numCount);
      setCount('');
    }
  };

  return (
    <form onSubmit={handleSubmit} className="flex gap-4 mb-6">
      <Input
        type="number"
        placeholder="Enter number of processes"
        value={count}
        onChange={(e) => setCount(e.target.value)}
        className="max-w-xs"
      />
      <Button type="submit">Generate Processes</Button>
    </form>
  );
};

// SortableTableHeader Component
const SortableTableHeader = ({ 
  column, 
  sortConfig, 
  onSort 
}: { 
  column: keyof Process;
  sortConfig: SortConfig | null;
  onSort: (key: keyof Process) => void;
}) => {
  const displayName = {
    id: 'Process ID',
    burstTime: 'Burst Time',
    arrivalTime: 'Arrival Time',
    priority: 'Priority'
  }[column];

  return (
    <TableHead 
      className="cursor-pointer hover:bg-gray-50"
      onClick={() => onSort(column)}
    >
      <div className="flex items-center justify-between">
        {displayName}
        <span className="ml-2">
          {sortConfig?.key === column ? (
            sortConfig.direction === 'asc' ? <ChevronUp size={16} /> : <ChevronDown size={16} />
          ) : (
            <ArrowUpDown size={16} className="opacity-50" />
          )}
        </span>
      </div>
    </TableHead>
  );
};

// ProcessTable Component
const ProcessTable = ({ 
  processes, 
  loading, 
  error 
}: { 
  processes: Process[];
  loading: boolean;
  error: string | null;
}) => {
  const [sortConfig, setSortConfig] = useState<SortConfig | null>(null);

  const getPriorityColor = (priority: number) => {
    if (priority <= 2) return "destructive";
    if (priority <= 4) return "secondary";
    return "default";
  };

  const handleSort = (key: keyof Process) => {
    setSortConfig((currentConfig) => {
      if (!currentConfig || currentConfig.key !== key) {
        return { key, direction: 'asc' };
      }
      if (currentConfig.direction === 'asc') {
        return { key, direction: 'desc' };
      }
      return null;
    });
  };

  const sortedProcesses = [...processes].sort((a, b) => {
    if (!sortConfig) return 0;
    
    const { key, direction } = sortConfig;
    const modifier = direction === 'asc' ? 1 : -1;
    
    return modifier * (a[key] < b[key] ? -1 : a[key] > b[key] ? 1 : 0);
  });

  if (loading) {
    return (
      <div className="space-y-2">
        {[...Array(5)].map((_, index) => (
          <Skeleton key={index} className="h-8 w-full" />
        ))}
      </div>
    );
  }

  if (error) {
    return <div className="text-red-500 font-medium py-4 text-center">{error}</div>;
  }

  return (
    <Table>
      <TableCaption>
        {sortConfig 
          ? `Processes sorted by ${sortConfig.key} (${sortConfig.direction})`
          : 'Click column headers to sort'}
      </TableCaption>
      <TableHeader>
        <TableRow>
          {(['id', 'burstTime', 'arrivalTime', 'priority'] as const).map((column) => (
            <SortableTableHeader
              key={column}
              column={column}
              sortConfig={sortConfig}
              onSort={handleSort}
            />
          ))}
        </TableRow>
      </TableHeader>
      <TableBody>
        {sortedProcesses.map((process) => (
          <TableRow key={process.id}>
            <TableCell className="font-medium">{process.id}</TableCell>
            <TableCell>{process.burstTime} </TableCell>
            <TableCell>{process.arrivalTime} </TableCell>
            <TableCell >
                {process.priority}
            </TableCell>
          </TableRow>
        ))}
      </TableBody>
    </Table>
  );
};

// Main CPU Scheduler Component
const CPUScheduler = () => {
  const [processes, setProcesses] = useState<Process[]>([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const handleGenerateProcesses = async (count: number) => {
    setLoading(true);
    setError(null);
    try {
      const response = await fetch(`/api/processes/${count}`);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const data = await response.json();
      setProcesses(data);
    } catch (error) {
      setError("Error generating processes. Please try again later.");
      console.error("Error:", error);
    } finally {
      setLoading(false);
    }
  };

  return (
    <Card className="w-full mx-auto">
      <CardHeader className="bg-gradient-to-r from-blue-500 to-purple-500 text-white">
        <CardTitle className="text-2xl text-center"> Welcome to CPU Scheduling Simulator</CardTitle>
      </CardHeader>
      <CardContent className="p-6">
        <ProcessInputForm onSubmit={handleGenerateProcesses} />
        <div className="flex justify-between items-center mb-4">
        <ProcessTable 
          processes={processes}
          loading={loading}
          error={error}
        />
        </div>
      </CardContent>
    </Card>
  );
};

export default CPUScheduler;