import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table"

interface Process {
  id: number
  arrivalTime: number
  burstTime: number
  priority: number
}

interface ProcessTableProps {
  processes: Process[]
}

export default function ProcessTable({ processes }: ProcessTableProps) {
  return (
    <Table>
      <TableHeader>
        <TableRow>
          <TableHead>Process ID</TableHead>
          <TableHead>Arrival Time</TableHead>
          <TableHead>Burst Time</TableHead>
          <TableHead>Priority</TableHead>
        </TableRow>
      </TableHeader>
      <TableBody>
        {processes.map((process) => (
          <TableRow key={process.id}>
            <TableCell>{process.id}</TableCell>
            <TableCell>{process.arrivalTime}</TableCell>
            <TableCell>{process.burstTime}</TableCell>
            <TableCell>{process.priority}</TableCell>
          </TableRow>
        ))}
      </TableBody>
    </Table>
  )
}

