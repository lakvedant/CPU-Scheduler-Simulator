#include <crow.h>
#include <random>
#include <vector>
#include <algorithm>
#include <queue>
#include <deque>

using namespace std;

struct Process {
    int id;
    int burstTime;
    int arrivalTime;
    int priority;

    int completionTime = 0;
    int turnaroundTime = 0;
    int waitingTime = 0;
    int responseTime = 0;
    bool started = false;
};

struct ProcessMetrics {
    int id;
    int burstTime;
    int arrivalTime;
    int priority;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    int responseTime;
};


struct GanttEntry {
    int processId;
    int startTime;
    int endTime;
};

struct AlgorithmResult {
    string name;
    vector<GanttEntry> ganttChart;
    vector<ProcessMetrics> processMetrics;
    double avgTurnaroundTime;
    double avgWaitingTime;
    double avgResponseTime;
    double throughput;
    double avgCompletionTime;
};

vector<Process> generateRandomProcesses(int count) {
    vector<Process> processes;
    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<> burstTimeDist(1, 100);
    uniform_int_distribution<> arrivalTimeDist(0, count < 10 ? 10 : count * 2);
    uniform_int_distribution<> priorityDist(1, count*2);

    for (int i = 1; i <= count; i++) {
        Process p;
        p.id = i;
        p.burstTime = burstTimeDist(gen);
        p.arrivalTime = arrivalTimeDist(gen);
        p.priority = priorityDist(gen);
        processes.push_back(p);
    }

    return processes;
}

AlgorithmResult calculateMetrics(const string& name,
                               const vector<Process>& processes,
                               const vector<GanttEntry>& ganttChart) {
    AlgorithmResult result;
    result.name = name;
    result.ganttChart = ganttChart;

    double totalTurnaroundTime = 0;
    double totalWaitingTime = 0;
    double totalResponseTime = 0;
    double totalCompletionTime = 0;

    for (const auto& p : processes) {
        ProcessMetrics pm;
        pm.id = p.id;
        pm.arrivalTime = p.arrivalTime;
        pm.burstTime = p.burstTime;
        pm.priority = p.priority;
        pm.completionTime = p.completionTime;
        pm.turnaroundTime = p.turnaroundTime;
        pm.waitingTime = p.waitingTime;
        pm.responseTime = p.responseTime;
        result.processMetrics.push_back(pm);

        totalTurnaroundTime += p.turnaroundTime;
        totalWaitingTime += p.waitingTime;
        totalResponseTime += p.responseTime;
        totalCompletionTime += p.completionTime;
    }

    int totalTime = ganttChart.empty() ? 0 : ganttChart.back().endTime;

    result.avgTurnaroundTime = totalTurnaroundTime / processes.size();
    result.avgWaitingTime = totalWaitingTime / processes.size();
    result.avgResponseTime = totalResponseTime / processes.size();
    result.avgCompletionTime = totalCompletionTime / processes.size();
    result.throughput = static_cast<double>(processes.size()) / totalTime;

    return result;
}

// (FCFS) Algorithm
AlgorithmResult fcfs(vector<Process> processes) {
    vector<GanttEntry> ganttChart;
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    int currentTime = 0;
    for (auto& p : processes) {
        if (currentTime < p.arrivalTime) {
            currentTime = p.arrivalTime;
        }

        GanttEntry entry;
        entry.processId = p.id;
        entry.startTime = currentTime;

        p.responseTime = currentTime - p.arrivalTime;
        currentTime += p.burstTime;
        entry.endTime = currentTime;
        ganttChart.push_back(entry);

        p.completionTime = currentTime;
        p.turnaroundTime = p.completionTime - p.arrivalTime;
        p.waitingTime = p.turnaroundTime - p.burstTime;
    }

    return calculateMetrics("FCFS", processes, ganttChart);
}

// (SJF) Algorithm
AlgorithmResult sjf(std::vector<Process> processes) {
    vector<GanttEntry> ganttChart;
    vector<Process*> remainingProcesses;

    for (auto& p : processes) {
        p.arrivalTime = 0;
        remainingProcesses.push_back(&p);
    }

    int currentTime = 0;

    sort(remainingProcesses.begin(), remainingProcesses.end(), 
        [](const Process* a, const Process* b) {
            return a->burstTime < b->burstTime;
    });

    for (auto* p : remainingProcesses) {
        GanttEntry entry;
        entry.processId = p->id;
        entry.startTime = currentTime;
        
        p->responseTime = currentTime;
        
        currentTime += p->burstTime;
        entry.endTime = currentTime;
        ganttChart.push_back(entry);

        p->completionTime = currentTime;
        p->turnaroundTime = p->completionTime;
        p->waitingTime = p->turnaroundTime - p->burstTime;
    }

    return calculateMetrics("SJF", processes, ganttChart);
}

AlgorithmResult srtn(vector<Process> processes) {
    vector<GanttEntry> ganttChart;
    vector<Process*> remainingProcesses;

    for (auto& p : processes) {
        remainingProcesses.push_back(&p);
    }

    std::sort(remainingProcesses.begin(), remainingProcesses.end(), [](const Process* a, const Process* b) {
        return a->arrivalTime < b->arrivalTime;
    });

    int currentTime = 0;
    Process* currentProcess = nullptr;

    vector<pair<Process*, int>> remainingBurstTimes;
    for (auto* p : remainingProcesses) {
        remainingBurstTimes.push_back({p, p->burstTime});
    }

    while (!remainingBurstTimes.empty()) {
        // Update available processes at current time
        vector<pair<Process*, int>> availableProcesses;
        for (const auto& pair : remainingBurstTimes) {
            if (pair.first->arrivalTime <= currentTime) {
                availableProcesses.push_back(pair);
            }
        }

        if (availableProcesses.empty()) {
            // Fast-forward to the next process arrival
            int nextArrival = INT_MAX;
            for (const auto& pair : remainingBurstTimes) {
                if (pair.first->arrivalTime < nextArrival) {
                    nextArrival = pair.first->arrivalTime;
                }
            }
            currentTime = nextArrival;
            continue;
        }

        // Find the process with shortest remaining time
        auto it = min_element(availableProcesses.begin(), availableProcesses.end(),
                                   [](const auto& a, const auto& b) {
                                       return a.second < b.second;
                                   });
        Process* selectedProcess = it->first;

        // Create a new gantt entry or extend the last one if it's the same process
        if (!ganttChart.empty() && ganttChart.back().processId == selectedProcess->id) {
            ganttChart.back().endTime = currentTime + 1;
        } else {
            if (!ganttChart.empty()) {
                ganttChart.back().endTime = currentTime;
            }
            GanttEntry entry;
            entry.processId = selectedProcess->id;
            entry.startTime = currentTime;
            entry.endTime = currentTime + 1;
            ganttChart.push_back(entry);

            if (!selectedProcess->started) {
                selectedProcess->responseTime = currentTime - selectedProcess->arrivalTime;
                selectedProcess->started = true;
            }
        }

        // Update the remaining burst time
        for (auto& pair : remainingBurstTimes) {
            if (pair.first == selectedProcess) {
                pair.second--;
                if (pair.second == 0) {
                    // Process completed
                    selectedProcess->completionTime = currentTime + 1;
                    selectedProcess->turnaroundTime = selectedProcess->completionTime - selectedProcess->arrivalTime;
                    selectedProcess->waitingTime = selectedProcess->turnaroundTime - selectedProcess->burstTime;

                    // Remove the completed process
                    remainingBurstTimes.erase(std::remove(remainingBurstTimes.begin(), remainingBurstTimes.end(), pair),
                                              remainingBurstTimes.end());
                    break;
                }
            }
        }

        currentTime++;
    }

    // Clean up gantt chart by merging consecutive entries with same process
    vector<GanttEntry> mergedGanttChart;
    for (const auto& entry : ganttChart) {
        if (mergedGanttChart.empty() || mergedGanttChart.back().processId != entry.processId) {
            mergedGanttChart.push_back(entry);
        } else {
            mergedGanttChart.back().endTime = entry.endTime;
        }
    }

    return calculateMetrics("SRTN", processes, mergedGanttChart);
}

// Round Robin (RR) Algorithm
AlgorithmResult roundRobin(std::vector<Process> processes, int timeQuantum) {
    vector<GanttEntry> ganttChart;
    deque<Process*> readyQueue;
    vector<Process*> remainingProcesses;

    for (auto& p : processes) {
        p.completionTime = 0;
        p.turnaroundTime = 0;
        p.waitingTime = 0;
        p.responseTime = -1;
        p.started = false;
        remainingProcesses.push_back(&p);
    }

    sort(remainingProcesses.begin(), remainingProcesses.end(), [](const Process* a, const Process* b) {
        return a->arrivalTime < b->arrivalTime;
    });

    vector<pair<Process*, int>> remainingBurstTimes;
    for (auto* p : remainingProcesses) {
        remainingBurstTimes.push_back({p, p->burstTime});
    }

    int currentTime = 0;

    // Add processes that arrive at time 0
    for (auto& pair : remainingBurstTimes) {
        if (pair.first->arrivalTime == 0) {
            readyQueue.push_back(pair.first);
        }
    }

    while (!readyQueue.empty() || !remainingBurstTimes.empty()) {
        // Check for new arrivals
        for (auto it = remainingBurstTimes.begin(); it != remainingBurstTimes.end();) {
            if (it->second > 0 && it->first->arrivalTime <= currentTime &&
                std::find(readyQueue.begin(), readyQueue.end(), it->first) == readyQueue.end()) {
                readyQueue.push_back(it->first);
            }
            if (it->second <= 0) {
                it = remainingBurstTimes.erase(it);
            } else {
                ++it;
            }
        }

        if (readyQueue.empty()) {
            // Find next arrival time
            int nextArrival = INT_MAX;
            for (const auto& pair : remainingBurstTimes) {
                if (pair.first->arrivalTime < nextArrival) {
                    nextArrival = pair.first->arrivalTime;
                }
            }
            currentTime = nextArrival;
            continue;
        }

        Process* currentProcess = readyQueue.front();
        readyQueue.pop_front();

        if (!currentProcess->started) {
            currentProcess->responseTime = currentTime - currentProcess->arrivalTime;
            currentProcess->started = true;
        }

        // Find remaining burst time
        int remainingBurst = 0;
        for (auto& pair : remainingBurstTimes) {
            if (pair.first == currentProcess) {
                remainingBurst = pair.second;
                break;
            }
        }

        int executionTime = std::min(timeQuantum, remainingBurst);

        GanttEntry entry;
        entry.processId = currentProcess->id;
        entry.startTime = currentTime;
        entry.endTime = currentTime + executionTime;
        ganttChart.push_back(entry);

        currentTime += executionTime;

        // Update remaining burst time
        for (auto& pair : remainingBurstTimes) {
            if (pair.first == currentProcess) {
                pair.second -= executionTime;

                if (pair.second <= 0) {
                    // Process completed
                    currentProcess->completionTime = currentTime;
                    currentProcess->turnaroundTime = currentProcess->completionTime - currentProcess->arrivalTime;
                    currentProcess->waitingTime = currentProcess->turnaroundTime - currentProcess->burstTime;
                } else {
                    // Process still has remaining time, add back to ready queue
                    // Check for new arrivals before re-adding the current process
                    for (auto it = remainingBurstTimes.begin(); it != remainingBurstTimes.end(); ++it) {
                        if (it->first != currentProcess && it->second > 0 &&
                            it->first->arrivalTime <= currentTime &&
                            std::find(readyQueue.begin(), readyQueue.end(), it->first) == readyQueue.end()) {
                            readyQueue.push_back(it->first);
                        }
                    }
                    readyQueue.push_back(currentProcess);
                }
                break;
            }
        }
    }

    // Clean up gantt chart by merging consecutive entries with same process
    std::vector<GanttEntry> mergedGanttChart;
    for (const auto& entry : ganttChart) {
        if (mergedGanttChart.empty() || mergedGanttChart.back().processId != entry.processId) {
            mergedGanttChart.push_back(entry);
        } else {
            mergedGanttChart.back().endTime = entry.endTime;
        }
    }

    return calculateMetrics("Round Robin (TQ=" + std::to_string(timeQuantum) + ")", processes, mergedGanttChart);
}

// Priority (Non-Preemptive) Algorithm
AlgorithmResult priorityNonPreemptive(std::vector<Process> processes) {
    std::vector<GanttEntry> ganttChart;
    std::vector<Process*> remainingProcesses;

    for (auto& p : processes) {
        remainingProcesses.push_back(&p);
    }

    int currentTime = 0;
    while (!remainingProcesses.empty()) {
        std::vector<Process*> availableProcesses;
        for (auto* p : remainingProcesses) {
            if (p->arrivalTime <= currentTime) {
                availableProcesses.push_back(p);
            }
        }

        if (availableProcesses.empty()) {
            // Fast-forward to the next process arrival
            int nextArrival = INT_MAX;
            for (auto* p : remainingProcesses) {
                if (p->arrivalTime < nextArrival) {
                    nextArrival = p->arrivalTime;
                }
            }
            currentTime = nextArrival;
            continue;
        }

        // Find the process with highest priority (lower number = higher priority)
        auto it = std::min_element(availableProcesses.begin(), availableProcesses.end(),
                                   [](const Process* a, const Process* b) {
                                       return a->priority < b->priority;
                                   });
        Process* selectedProcess = *it;

        GanttEntry entry;
        entry.processId = selectedProcess->id;
        entry.startTime = currentTime;

        selectedProcess->responseTime = currentTime - selectedProcess->arrivalTime;
        currentTime += selectedProcess->burstTime;
        entry.endTime = currentTime;
        ganttChart.push_back(entry);

        selectedProcess->completionTime = currentTime;
        selectedProcess->turnaroundTime = selectedProcess->completionTime - selectedProcess->arrivalTime;
        selectedProcess->waitingTime = selectedProcess->turnaroundTime - selectedProcess->burstTime;

        // Remove the selected process from remaining processes
        remainingProcesses.erase(std::remove(remainingProcesses.begin(), remainingProcesses.end(), selectedProcess),
                                 remainingProcesses.end());
    }

    return calculateMetrics("Priority (Non-Preemptive)", processes, ganttChart);
}

// Priority (Preemptive) Algorithm
AlgorithmResult priorityPreemptive(std::vector<Process> processes) {
    std::vector<GanttEntry> ganttChart;
    std::vector<Process*> remainingProcesses;

    for (auto& p : processes) {
        remainingProcesses.push_back(&p);
        p.started = false;
    }

    std::sort(remainingProcesses.begin(), remainingProcesses.end(), [](const Process* a, const Process* b) {
        return a->arrivalTime < b->arrivalTime;
    });

    std::vector<std::pair<Process*, int>> remainingBurstTimes;
    for (auto* p : remainingProcesses) {
        remainingBurstTimes.push_back({p, p->burstTime});
    }

    int currentTime = 0;
    Process* currentProcess = nullptr;

    while (!remainingBurstTimes.empty()) {
        // Update available processes at current time
        std::vector<std::pair<Process*, int>> availableProcesses;
        for (const auto& pair : remainingBurstTimes) {
            if (pair.first->arrivalTime <= currentTime) {
                availableProcesses.push_back(pair);
            }
        }

        if (availableProcesses.empty()) {
            // Fast-forward to the next process arrival
            int nextArrival = INT_MAX;
            for (const auto& pair : remainingBurstTimes) {
                if (pair.first->arrivalTime < nextArrival) {
                    nextArrival = pair.first->arrivalTime;
                }
            }
            currentTime = nextArrival;
            continue;
        }

        // Find the process with highest priority
        auto it = std::min_element(availableProcesses.begin(), availableProcesses.end(),
                                   [](const auto& a, const auto& b) {
                                       return a.first->priority < b.first->priority;
                                   });
        Process* selectedProcess = it->first;

        // Create a new gantt entry or extend the last one if it's the same process
        if (!ganttChart.empty() && ganttChart.back().processId == selectedProcess->id) {
            ganttChart.back().endTime = currentTime + 1;
        } else {
            if (!ganttChart.empty()) {
                ganttChart.back().endTime = currentTime;
            }
            GanttEntry entry;
            entry.processId = selectedProcess->id;
            entry.startTime = currentTime;
            entry.endTime = currentTime + 1;
            ganttChart.push_back(entry);

            if (!selectedProcess->started) {
                selectedProcess->responseTime = currentTime - selectedProcess->arrivalTime;
                selectedProcess->started = true;
            }
        }

        // Update the remaining burst time
        for (auto& pair : remainingBurstTimes) {
            if (pair.first == selectedProcess) {
                pair.second--;
                if (pair.second == 0) {
                    // Process completed
                    selectedProcess->completionTime = currentTime + 1;
                    selectedProcess->turnaroundTime = selectedProcess->completionTime - selectedProcess->arrivalTime;
                    selectedProcess->waitingTime = selectedProcess->turnaroundTime - selectedProcess->burstTime;

                    // Remove the completed process
                    remainingBurstTimes.erase(std::remove(remainingBurstTimes.begin(), remainingBurstTimes.end(), pair),
                                              remainingBurstTimes.end());
                    break;
                }
            }
        }

        currentTime++;
    }

    // Clean up gantt chart by merging consecutive entries with same process
    vector<GanttEntry> mergedGanttChart;
    for (const auto& entry : ganttChart) {
        if (mergedGanttChart.empty() || mergedGanttChart.back().processId != entry.processId) {
            mergedGanttChart.push_back(entry);
        } else {
            mergedGanttChart.back().endTime = entry.endTime;
        }
    }

    return calculateMetrics("Priority (Preemptive)", processes, mergedGanttChart);
}

// Middleware for handling CORS
struct CORSMiddleware {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }

    void after_handle(crow::request&, crow::response&, context&) {}
};

int main() {
    // Create Crow app with CORS middleware
    crow::App<CORSMiddleware> app;

    // Generate processes
    CROW_ROUTE(app, "/api/processes/<int>")
    ([](int count) {
        auto processes = generateRandomProcesses(count);

        crow::json::wvalue response = crow::json::wvalue::list();
        for (size_t i = 0; i < processes.size(); i++) {
            crow::json::wvalue process;
            process["id"] = processes[i].id;
            process["burstTime"] = processes[i].burstTime;
            process["arrivalTime"] = processes[i].arrivalTime;
            process["priority"] = processes[i].priority;
            response[i] = std::move(process);
        }

        crow::response res(response);
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // Run algorithms on processes
    CROW_ROUTE(app, "/api/schedule").methods("POST"_method)
    ([](const crow::request& req) {
        auto params = crow::json::load(req.body);

        if (!params) {
            return crow::response(400, "Invalid JSON");
        }

        vector<Process> processes;
        for (const auto& item : params["processes"]) {
            Process p;
            p.id = item["id"].i();
            p.burstTime = item["burstTime"].i();
            p.arrivalTime = item["arrivalTime"].i();
            p.priority = item["priority"].i();
            processes.push_back(p);
        }

        int timeQuantum = 2;
        if (params.has("timeQuantum")) {
            timeQuantum = params["timeQuantum"].i();
        }

        vector<AlgorithmResult> results;

        // Run each selected algorithm on a fresh copy of the processes
        if (params["algorithms"].has("fcfs") && params["algorithms"]["fcfs"].b()) {
            vector<Process> processesCopy = processes;
            results.push_back(fcfs(processesCopy));
        }
        if (params["algorithms"].has("sjf") && params["algorithms"]["sjf"].b()) {
            vector<Process> processesCopy = processes;
            results.push_back(sjf(processesCopy));
        }
        if (params["algorithms"].has("srtn") && params["algorithms"]["srtn"].b()) {
            vector<Process> processesCopy = processes;
            results.push_back(srtn(processesCopy));
        }
        if (params["algorithms"].has("roundRobin") && params["algorithms"]["roundRobin"].b()) {
            vector<Process> processesCopy = processes;
            results.push_back(roundRobin(processesCopy, timeQuantum));
        }
        if (params["algorithms"].has("priority") && params["algorithms"]["priority"].b()) {
            vector<Process> processesCopy = processes;
            results.push_back(priorityNonPreemptive(processesCopy));
        }
        if (params["algorithms"].has("priorityPreemptive") && params["algorithms"]["priorityPreemptive"].b()) {
            vector<Process> processesCopy = processes;
            results.push_back(priorityPreemptive(processesCopy));
        }

        // Create response JSON
        crow::json::wvalue response = crow::json::wvalue::list();
        for (size_t i = 0; i < results.size(); i++) {
            crow::json::wvalue result;
            result["name"] = results[i].name;
            result["avgTurnaroundTime"] = results[i].avgTurnaroundTime;
            result["avgWaitingTime"] = results[i].avgWaitingTime;
            result["avgResponseTime"] = results[i].avgResponseTime;
            result["avgCompletionTime"] = results[i].avgCompletionTime;
            result["throughput"] = results[i].throughput;

            // Add gantt chart data
            crow::json::wvalue ganttChart = crow::json::wvalue::list();
            for (size_t j = 0; j < results[i].ganttChart.size(); j++) {
                crow::json::wvalue entry;
                entry["processId"] = results[i].ganttChart[j].processId;
                entry["startTime"] = results[i].ganttChart[j].startTime;
                entry["endTime"] = results[i].ganttChart[j].endTime;
                ganttChart[j] = std::move(entry);
            }
            result["ganttChart"] = std::move(ganttChart);

            // Add process-specific metrics
            crow::json::wvalue processMetrics = crow::json::wvalue::list();
            for (const auto& pm : results[i].processMetrics) {
                crow::json::wvalue processResult;
                processResult["id"] = pm.id;
                processResult["arrivalTime"] = pm.arrivalTime;
                processResult ["burstTime"] = pm.burstTime;
                 processResult ["priority"] = pm.priority;
                processResult["completionTime"] = pm.completionTime;
                processResult["turnaroundTime"] = pm.turnaroundTime;
                processResult["waitingTime"] = pm.waitingTime;
                processResult["responseTime"] = pm.responseTime;
                processMetrics[processMetrics.size()] = std::move(processResult);
            }
            result["processes"] = std::move(processMetrics);

            response[i] = std::move(result);
        }

        crow::response res(response);
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // Run the app on port 8080
    app.port(8080).run();
    return 0;
}
