#include <crow.h>
#include <random>
#include <vector>
#include <algorithm>
#include <queue>
#include <deque>

using namespace std;

// Add CORS middleware definition
struct CORSMiddleware {
    struct context {};
    
    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
    }

    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        // Nothing to do
    }
};

struct Process {
    int id, burstTime, arrivalTime, priority;
    int completionTime = 0, turnaroundTime = 0, waitingTime = 0, responseTime = 0;
    bool started = false;
};

struct GanttEntry {
    int processId, startTime, endTime;
};

struct AlgorithmResult {
    string name;
    vector<GanttEntry> ganttChart;
    vector<Process> processMetrics;
    double avgTurnaroundTime, avgWaitingTime, avgResponseTime, throughput, avgCompletionTime;
};

vector<Process> generateRandomProcesses(int count) {
    vector<Process> processes;
    random_device rd;
    mt19937 gen(rd());
    
    for (int i = 1; i <= count; i++) {
        processes.push_back({
            i,
            uniform_int_distribution<>(1, 100)(gen),
            uniform_int_distribution<>(0, count < 10 ? 10 : count * 2)(gen),
            uniform_int_distribution<>(1, count*2)(gen)
        });
    }
    return processes;
}

AlgorithmResult calculateMetrics(const string& name, const vector<Process>& processes, const vector<GanttEntry>& ganttChart) {
    AlgorithmResult result{name, ganttChart, processes};
    double totalTurnaround = 0, totalWaiting = 0, totalResponse = 0, totalCompletion = 0;
    
    for (const auto& p : processes) {
        totalTurnaround += p.turnaroundTime;
        totalWaiting += p.waitingTime;
        totalResponse += p.responseTime;
        totalCompletion += p.completionTime;
    }
    
    int procCount = processes.size();
    int totalTime = ganttChart.empty() ? 0 : ganttChart.back().endTime;
    
    result.avgTurnaroundTime = totalTurnaround / procCount;
    result.avgWaitingTime = totalWaiting / procCount;
    result.avgResponseTime = totalResponse / procCount;
    result.avgCompletionTime = totalCompletion / procCount;
    result.throughput = static_cast<double>(procCount) / totalTime;
    
    return result;
}

AlgorithmResult fcfs(vector<Process> processes) {
    vector<GanttEntry> ganttChart;
    sort(processes.begin(), processes.end(), 
         [](const Process& a, const Process& b) { return a.arrivalTime < b.arrivalTime; });
    
    int currentTime = 0;
    for (auto& p : processes) {
        currentTime = max(currentTime, p.arrivalTime);
        ganttChart.push_back({p.id, currentTime, currentTime + p.burstTime});
        
        p.responseTime = currentTime - p.arrivalTime;
        p.completionTime = currentTime + p.burstTime;
        p.turnaroundTime = p.completionTime - p.arrivalTime;
        p.waitingTime = p.turnaroundTime - p.burstTime;
        
        currentTime = p.completionTime;
    }
    
    return calculateMetrics("FCFS", processes, ganttChart);
}

AlgorithmResult sjf(vector<Process> processes) {
    vector<GanttEntry> ganttChart;
    
    for (auto& p : processes) p.arrivalTime = 0;
    sort(processes.begin(), processes.end(), 
         [](const Process& a, const Process& b) { return a.burstTime < b.burstTime; });
    
    int currentTime = 0;
    for (auto& p : processes) {
        ganttChart.push_back({p.id, currentTime, currentTime + p.burstTime});
        
        p.responseTime = currentTime;
        p.completionTime = currentTime + p.burstTime;
        p.turnaroundTime = p.completionTime;
        p.waitingTime = p.turnaroundTime - p.burstTime;
        
        currentTime = p.completionTime;
    }
    
    return calculateMetrics("SJF", processes, ganttChart);
}

AlgorithmResult srtn(vector<Process> processes) {
    vector<GanttEntry> ganttChart;
    vector<pair<Process*, int>> remaining;
    
    for (auto& p : processes) {
        remaining.push_back({&p, p.burstTime});
    }
    
    int currentTime = 0;
    while (!remaining.empty()) {
        auto availableProcs = vector<pair<Process*, int>>();
        for (const auto& [proc, burst] : remaining) {
            if (proc->arrivalTime <= currentTime) availableProcs.push_back({proc, burst});
        }
        
        if (availableProcs.empty()) {
            auto minArrival = min_element(remaining.begin(), remaining.end(),
                [](const auto& a, const auto& b) { 
                    return a.first->arrivalTime < b.first->arrivalTime; 
                });
            currentTime = minArrival->first->arrivalTime;
            continue;
        }
        
        auto it = min_element(availableProcs.begin(), availableProcs.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        Process* selected = it->first;
        
        if (!ganttChart.empty() && ganttChart.back().processId == selected->id) {
            ganttChart.back().endTime = currentTime + 1;
        } else {
            if (!ganttChart.empty()) ganttChart.back().endTime = currentTime;
            ganttChart.push_back({selected->id, currentTime, currentTime + 1});
            
            if (!selected->started) {
                selected->responseTime = currentTime - selected->arrivalTime;
                selected->started = true;
            }
        }
        
        auto& [_, remainingBurst] = *find_if(remaining.begin(), remaining.end(),
            [selected](const auto& p) { return p.first == selected; });
        
        remainingBurst--;
        if (remainingBurst == 0) {
            selected->completionTime = currentTime + 1;
            selected->turnaroundTime = selected->completionTime - selected->arrivalTime;
            selected->waitingTime = selected->turnaroundTime - selected->burstTime;
            remaining.erase(find_if(remaining.begin(), remaining.end(),
                [selected](const auto& p) { return p.first == selected; }));
        }
        
        currentTime++;
    }
    
    return calculateMetrics("SRTN", processes, ganttChart);
}

AlgorithmResult roundRobin(vector<Process> processes, int timeQuantum) {
    vector<GanttEntry> ganttChart;
    deque<Process*> queue;
    vector<pair<Process*, int>> remaining;
    
    for (auto& p : processes) {
        remaining.push_back({&p, p.burstTime});
        if (p.arrivalTime == 0) queue.push_back(&p);
    }
    
    int currentTime = 0;
    while (!queue.empty() || !remaining.empty()) {
        for (auto& [proc, burst] : remaining) {
            if (burst > 0 && proc->arrivalTime <= currentTime && 
                find(queue.begin(), queue.end(), proc) == queue.end()) {
                queue.push_back(proc);
            }
        }
        
        if (queue.empty()) {
            auto minArrival = min_element(remaining.begin(), remaining.end(),
                [](const auto& a, const auto& b) { 
                    return a.first->arrivalTime < b.first->arrivalTime; 
                });
            currentTime = minArrival->first->arrivalTime;
            continue;
        }
        
        Process* current = queue.front();
        queue.pop_front();
        
        if (!current->started) {
            current->responseTime = currentTime - current->arrivalTime;
            current->started = true;
        }
        
        auto& [_, remainingBurst] = *find_if(remaining.begin(), remaining.end(),
            [current](const auto& p) { return p.first == current; });
        
        int execTime = min(timeQuantum, remainingBurst);
        ganttChart.push_back({current->id, currentTime, currentTime + execTime});
        
        remainingBurst -= execTime;
        currentTime += execTime;
        
        if (remainingBurst > 0) queue.push_back(current);
        else {
            current->completionTime = currentTime;
            current->turnaroundTime = currentTime - current->arrivalTime;
            current->waitingTime = current->turnaroundTime - current->burstTime;
        }
    }
    
    return calculateMetrics("Round Robin (TQ=" + to_string(timeQuantum) + ")", processes, ganttChart);
}

AlgorithmResult priorityNonPreemptive(vector<Process> processes) {
    vector<GanttEntry> ganttChart;
    vector<Process*> remaining;
    for (auto& p : processes) remaining.push_back(&p);
    
    int currentTime = 0;
    while (!remaining.empty()) {
        vector<Process*> available;
        for (auto* p : remaining) {
            if (p->arrivalTime <= currentTime) available.push_back(p);
        }
        
        if (available.empty()) {
            auto minArrival = min_element(remaining.begin(), remaining.end(),
                [](const Process* a, const Process* b) { 
                    return a->arrivalTime < b->arrivalTime; 
                });
            currentTime = (*minArrival)->arrivalTime;
            continue;
        }
        
        auto selected = *min_element(available.begin(), available.end(),
            [](const Process* a, const Process* b) { return a->priority < b->priority; });
        
        ganttChart.push_back({selected->id, currentTime, currentTime + selected->burstTime});
        
        selected->responseTime = currentTime - selected->arrivalTime;
        selected->completionTime = currentTime + selected->burstTime;
        selected->turnaroundTime = selected->completionTime - selected->arrivalTime;
        selected->waitingTime = selected->turnaroundTime - selected->burstTime;
        
        currentTime = selected->completionTime;
        remaining.erase(find(remaining.begin(), remaining.end(), selected));
    }
    
    return calculateMetrics("Priority (Non-Preemptive)", processes, ganttChart);
}

AlgorithmResult priorityPreemptive(vector<Process> processes) {
    vector<GanttEntry> ganttChart;
    vector<pair<Process*, int>> remaining;
    
    for (auto& p : processes) {
        remaining.push_back({&p, p.burstTime});
        p.started = false;
    }
    
    int currentTime = 0;
    while (!remaining.empty()) {
        vector<pair<Process*, int>> available;
        for (const auto& [proc, burst] : remaining) {
            if (proc->arrivalTime <= currentTime) available.push_back({proc, burst});
        }
        
        if (available.empty()) {
            auto minArrival = min_element(remaining.begin(), remaining.end(),
                [](const auto& a, const auto& b) { 
                    return a.first->arrivalTime < b.first->arrivalTime; 
                });
            currentTime = minArrival->first->arrivalTime;
            continue;
        }
        
        auto it = min_element(available.begin(), available.end(),
            [](const auto& a, const auto& b) { return a.first->priority < b.first->priority; });
        Process* selected = it->first;
        
        if (!ganttChart.empty() && ganttChart.back().processId == selected->id) {
            ganttChart.back().endTime = currentTime + 1;
        } else {
            if (!ganttChart.empty()) ganttChart.back().endTime = currentTime;
            ganttChart.push_back({selected->id, currentTime, currentTime + 1});
            
            if (!selected->started) {
                selected->responseTime = currentTime - selected->arrivalTime;
                selected->started = true;
            }
        }
        
        auto& [_, remainingBurst] = *find_if(remaining.begin(), remaining.end(),
            [selected](const auto& p) { return p.first == selected; });
            
        remainingBurst--;
        if (remainingBurst == 0) {
            selected->completionTime = currentTime + 1;
            selected->turnaroundTime = selected->completionTime - selected->arrivalTime;
            selected->waitingTime = selected->turnaroundTime - selected->burstTime;
            remaining.erase(find_if(remaining.begin(), remaining.end(),
                [selected](const auto& p) { return p.first == selected; }));
        }
        
        currentTime++;
    }
    
    return calculateMetrics("Priority (Preemptive)", processes, ganttChart);
}

int main() {
    crow::App<CORSMiddleware> app;
    
    CROW_ROUTE(app, "/api/processes/<int>")
    ([](int count) {
        auto processes = generateRandomProcesses(count);
        crow::json::wvalue response = crow::json::wvalue::list();
        
        for (const auto& p : processes) {
            response[response.size()] = {
                {"id", p.id},
                {"burstTime", p.burstTime},
                {"arrivalTime", p.arrivalTime},
                {"priority", p.priority}
            };
        }
        
        return crow::response(response);
    });
    
    CROW_ROUTE(app, "/api/schedule").methods("POST"_method)
    ([](const crow::request& req) {
        auto params = crow::json::load(req.body);
        if (!params) return crow::response(400, "Invalid JSON");
        
        vector<Process> processes;
        for (const auto& item : params["processes"]) {
            processes.push_back({
                static_cast<int>(item["id"].i()),
                static_cast<int>(item["burstTime"].i()),
                static_cast<int>(item["arrivalTime"].i()),
                static_cast<int>(item["priority"].i())
            });
        }
        
        vector<AlgorithmResult> results;
        if (params["algorithms"]["fcfs"].b()) 
            results.push_back(fcfs(processes));
        if (params["algorithms"]["sjf"].b())
            results.push_back(sjf(processes));
        if (params["algorithms"]["srtn"].b())
            results.push_back(srtn(processes));
        if (params["algorithms"]["roundRobin"].b())
            results.push_back(roundRobin(processes, params["timeQuantum"].i()));
        if (params["algorithms"]["priority"].b())
            results.push_back(priorityNonPreemptive(processes));
        if (params["algorithms"]["priorityPreemptive"].b())
            results.push_back(priorityPreemptive(processes));
        
        crow::json::wvalue response = crow::json::wvalue::list();
        for (const auto& result : results) {
            response[response.size()] = {
                {"name", result.name},
                {"avgTurnaroundTime", result.avgTurnaroundTime},
                {"avgWaitingTime", result.avgWaitingTime},
                {"avgResponseTime", result.avgResponseTime},
                {"avgCompletionTime", result.avgCompletionTime},
                {"throughput", result.throughput},
                {"ganttChart", [&result]() {
                    crow::json::wvalue chart = crow::json::wvalue::list();
                    for (const auto& entry : result.ganttChart) {
                        chart[chart.size()] = {
                            {"processId", entry.processId},
                            {"startTime", entry.startTime},
                            {"endTime", entry.endTime}
                        };
                    }
                    return chart;
                }()},
                {"processes", [&result]() {
                    crow::json::wvalue procs = crow::json::wvalue::list();
                    for (const auto& p : result.processMetrics) {
                        procs[procs.size()] = {
                            {"id", p.id},
                            {"arrivalTime", p.arrivalTime},
                            {"burstTime", p.burstTime},
                            {"priority", p.priority},
                            {"completionTime", p.completionTime},
                            {"turnaroundTime", p.turnaroundTime},
                            {"waitingTime", p.waitingTime},
                            {"responseTime", p.responseTime}
                        };
                    }
                    return procs;
                }()}
            };
        }
        
        return crow::response(response);
    });

    // Add OPTIONS route for CORS preflight requests
    CROW_ROUTE(app, "/api/schedule").methods("OPTIONS"_method)
    ([](const crow::request& req) {
        crow::response res;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        return res;
    });
    
    app.port(8080).run();
    return 0;
}