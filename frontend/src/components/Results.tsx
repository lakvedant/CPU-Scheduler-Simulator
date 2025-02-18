import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table"

interface AlgorithmResult {
  name: string
  avgTurnaroundTime: number
  avgWaitingTime: number
  avgResponseTime: number
  cpuUtilization: number
  throughput: number
}

interface ResultsTableProps {
  results: AlgorithmResult[]
}

export default function ResultsTable({ results }: ResultsTableProps) {
  return (
    <Table>
      <TableHeader>
        <TableRow>
          <TableHead>Algorithm</TableHead>
          <TableHead>Avg Turnaround Time</TableHead>
          <TableHead>Avg Waiting Time</TableHead>
          <TableHead>Avg Response Time</TableHead>
          <TableHead>CPU Utilization</TableHead>
          <TableHead>Throughput</TableHead>
        </TableRow>
      </TableHeader>
      <TableBody>
        {results.map((result, index) => (
          <TableRow key={index}>
            <TableCell>{result.name}</TableCell>
            <TableCell>{result.avgTurnaroundTime.toFixed(2)}</TableCell>
            <TableCell>{result.avgWaitingTime.toFixed(2)}</TableCell>
            <TableCell>{result.avgResponseTime.toFixed(2)}</TableCell>
            <TableCell>{result.cpuUtilization.toFixed(2)}%</TableCell>
            <TableCell>{result.throughput.toFixed(2)}</TableCell>
          </TableRow>
        ))}
      </TableBody>
    </Table>
  )
}

