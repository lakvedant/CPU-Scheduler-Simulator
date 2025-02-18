"use client"

import { useState } from "react"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Checkbox } from "@/components/ui/checkbox"
import { Label } from "@/components/ui/label"
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from "@/components/ui/card"
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs"
import { Separator } from "@/components/ui/separator"
import ProcessTable from "@/components/ProcessTable"
import GanttChart from "@/components/GranttChart"
import ResultsTable from "@/components/Results"

interface Process {
  id: number
  arrivalTime: number
  burstTime: number
  priority: number
}

interface AlgorithmResult {
  name: string
  avgTurnaroundTime: number
  avgWaitingTime: number
  avgResponseTime: number
  cpuUtilization: number
  throughput: number
  ganttChart: { processId: number; startTime: number; endTime: number }[]
}

export default function Complete() {
  const [processes, setProcesses] = useState<Process[]>([])
  const [processCount, setProcessCount] = useState(5)
  const [timeQuantum, setTimeQuantum] = useState(2)
  const [results, setResults] = useState<AlgorithmResult[]>([])
  const [algorithms, setAlgorithms] = useState({
    fcfs: true,
    sjf: false,
    srtn: false,
    roundRobin: false,
    priority: false,
    priorityPreemptive: false,
  })
  const [loading, setLoading] = useState(false)

  const generateProcesses = async () => {
    try {
      setLoading(true)
      const response = await fetch(`api/processes/${processCount}`)
      const data = await response.json()
      setProcesses(data)
    } catch (error) {
      console.error("Failed to generate processes:", error)
    } finally {
      setLoading(false)
    }
  }

  const runSimulation = async () => {
    try {
      setLoading(true)
      const response = await fetch("api/schedule", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          processes,
          algorithms,
          timeQuantum,
        }),
      })
      const data = await response.json()
      setResults(data)
    } catch (error) {
      console.error("Failed to run simulation:", error)
    } finally {
      setLoading(false)
    }
  }

  const algorithmLabels = {
    fcfs: "First Come First Serve",
    sjf: "Shortest Job First",
    srtn: "Shortest Remaining Time Next",
    roundRobin: "Round Robin",
    priority: "Priority (Non-preemptive)",
    priorityPreemptive: "Priority (Preemptive)",
  }

  return (
    <div className="container mx-auto py-8 px-4 ">
      <Card className="border-none shadow-lg bg-gradient-to-r from-blue-50 to-indigo-50">
        <CardHeader className="text-center pb-2">
          <CardTitle className="text-4xl font-bold text-indigo-800">CPU Scheduling Simulator</CardTitle>
          <CardDescription className="text-indigo-600 font-medium">
            Simulate and compare various CPU scheduling algorithms
          </CardDescription>
        </CardHeader>
        <CardContent>
          <div className="bg-white rounded-lg p-6 shadow-md">
            <div className="flex flex-col md:flex-row md:items-end gap-4 mb-6">
              <div className="w-full md:w-1/3">
                <Label htmlFor="processCount" className="font-medium text-gray-700 mb-1 block">
                  Number of Processes
                </Label>
                <div className="flex items-center gap-2">
                  <Input
                    id="processCount"
                    type="number"
                    value={processCount}
                    onChange={(e) => setProcessCount(Number.parseInt(e.target.value) || 1)}
                    className="w-full"
                    min="1"
                    max="20"
                  />
                  <Button 
                    onClick={generateProcesses} 
                    className="bg-indigo-600 hover:bg-indigo-700 text-white"
                    disabled={loading}
                  >
                    {loading ? "Generating..." : "Generate"}
                  </Button>
                </div>
              </div>
            </div>

            {processes.length > 0 && (
              <>
                <div className="mb-8">
                  <h2 className="text-xl font-semibold text-indigo-800 mb-4">Process Table</h2>
                  <div className="bg-gray-50 p-4 rounded-lg border border-gray-200">
                    <ProcessTable processes={processes} />
                  </div>
                </div>

                <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
                  <Card className="col-span-1 md:col-span-2">
                    <CardHeader>
                      <CardTitle className="text-lg font-semibold text-indigo-800">
                        Select Algorithms
                      </CardTitle>
                    </CardHeader>
                    <CardContent>
                      <div className="grid grid-cols-1 md:grid-cols-2 gap-3">
                        {Object.entries(algorithms).map(([key, value]) => (
                          <div key={key} className="flex items-center space-x-2 p-2 rounded hover:bg-gray-50">
                            <Checkbox
                              id={key}
                              checked={value}
                              onCheckedChange={(checked) => 
                                setAlgorithms((prev) => ({ ...prev, [key]: !!checked }))
                              }
                              className="text-indigo-600"
                            />
                            <Label htmlFor={key} className="font-medium cursor-pointer">
                              {algorithmLabels[key as keyof typeof algorithmLabels]}
                            </Label>
                          </div>
                        ))}
                      </div>
                    </CardContent>
                  </Card>

                  {algorithms.roundRobin && (
                    <Card>
                      <CardHeader>
                        <CardTitle className="text-lg font-semibold text-indigo-800">
                          Round Robin Settings
                        </CardTitle>
                      </CardHeader>
                      <CardContent>
                        <div className="space-y-2">
                          <Label htmlFor="timeQuantum" className="font-medium">Time Quantum</Label>
                          <Input
                            id="timeQuantum"
                            type="number"
                            value={timeQuantum}
                            onChange={(e) => setTimeQuantum(Number.parseInt(e.target.value) || 1)}
                            className="w-full"
                            min="1"
                          />
                          <p className="text-sm text-gray-500 mt-1">
                            Time slice allocated to each process
                          </p>
                        </div>
                      </CardContent>
                    </Card>
                  )}
                </div>

                <div className="flex justify-center mb-8">
                  <Button 
                    onClick={runSimulation}
                    className="bg-indigo-600 hover:bg-indigo-700 text-white font-semibold px-8 py-2 text-lg shadow-md transition-transform transform hover:scale-105"
                    disabled={loading}
                  >
                    {loading ? "Simulating..." : "Run Simulation"}
                  </Button>
                </div>
              </>
            )}
          </div>

          {results.length > 0 && (
            <div className="mt-10 bg-white rounded-lg p-6 shadow-md">
              <h2 className="text-2xl font-bold text-indigo-800 mb-6 flex items-center gap-2">
                <svg xmlns="http://www.w3.org/2000/svg" className="h-6 w-6" viewBox="0 0 20 20" fill="currentColor">
                  <path fillRule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-7-4a1 1 0 11-2 0 1 1 0 012 0zM9 9a1 1 0 000 2h2a1 1 0 100-2H9z" clipRule="evenodd" />
                </svg>
                Simulation Results
              </h2>

              <Tabs defaultValue="comparative">
                <TabsList className="w-full bg-gray-100 p-1 mb-6">
                  <TabsTrigger value="comparative" className="flex-1 font-medium">Comparison</TabsTrigger>
                  <TabsTrigger value="individual" className="flex-1 font-medium">Individual Results</TabsTrigger>
                </TabsList>

                <TabsContent value="comparative">
                  <div className="p-4 bg-gray-50 rounded-lg border border-gray-200 mb-6">
                    <h3 className="text-lg font-semibold text-indigo-800 mb-4">Algorithm Comparison</h3>
                    <ResultsTable results={results} />
                  </div>
                </TabsContent>

                <TabsContent value="individual">
                  <div className="space-y-8">
                    {results.map((result, index) => (
                      <Card key={index} className="overflow-hidden bg-white border-l-4" style={{ borderLeftColor: `hsl(${index * 60}, 70%, 60%)` }}>
                        <CardHeader className="bg-gray-50">
                          <CardTitle className="flex items-center gap-2 text-xl text-indigo-800">
                            <span className="inline-flex items-center justify-center w-8 h-8 rounded-full bg-indigo-100 text-indigo-800 font-bold text-sm">
                              {index + 1}
                            </span>
                            {result.name}
                          </CardTitle>
                        </CardHeader>
                        <CardContent className="pt-4">
                          <div className="mb-4">
                            <h4 className="text-lg font-semibold text-gray-700 mb-2">Gantt Chart</h4>
                            <div className="p-3 bg-gray-50 rounded border border-gray-200">
                              <GanttChart data={result.ganttChart} />
                            </div>
                          </div>
                          
                          <Separator className="my-4" />
                          
                          <div>
                            <h4 className="text-lg font-semibold text-gray-700 mb-2">Metrics</h4>
                            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
                              <div className="p-3 rounded-lg bg-blue-50 border border-blue-100">
                                <p className="text-sm font-medium text-blue-800">Average Turnaround Time</p>
                                <p className="text-2xl font-bold text-blue-900">{result.avgTurnaroundTime.toFixed(2)}</p>
                              </div>
                              <div className="p-3 rounded-lg bg-green-50 border border-green-100">
                                <p className="text-sm font-medium text-green-800">Average Waiting Time</p>
                                <p className="text-2xl font-bold text-green-900">{result.avgWaitingTime.toFixed(2)}</p>
                              </div>
                              <div className="p-3 rounded-lg bg-purple-50 border border-purple-100">
                                <p className="text-sm font-medium text-purple-800">Average Response Time</p>
                                <p className="text-2xl font-bold text-purple-900">{result.avgResponseTime.toFixed(2)}</p>
                              </div>
                              <div className="p-3 rounded-lg bg-amber-50 border border-amber-100">
                                <p className="text-sm font-medium text-amber-800">CPU Utilization</p>
                                <p className="text-2xl font-bold text-amber-900">{result.cpuUtilization.toFixed(2)}%</p>
                              </div>
                              <div className="p-3 rounded-lg bg-indigo-50 border border-indigo-100 sm:col-span-2">
                                <p className="text-sm font-medium text-indigo-800">Throughput</p>
                                <p className="text-2xl font-bold text-indigo-900">{result.throughput.toFixed(2)} processes/unit time</p>
                              </div>
                            </div>
                          </div>
                        </CardContent>
                      </Card>
                    ))}
                  </div>
                </TabsContent>
              </Tabs>
            </div>
          )}
        </CardContent>
        <CardFooter className="flex justify-center pt-0 pb-6">
          <p className="text-sm text-gray-500">
            © {new Date().getFullYear()} CPU Scheduling Simulator | All Rights Reserved
          </p>
        </CardFooter>
      </Card>
    </div>
  )
}