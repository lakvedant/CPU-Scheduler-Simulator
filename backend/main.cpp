#include <crow.h>
#include <random>
#include <vector>

struct Process {
    int id;
    int burstTime;
    int arrivalTime;
    int priority;
};

std::vector<Process> generateRandomProcesses(int count) {
    std::vector<Process> processes;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_int_distribution<> burstTimeDist(1, 20);
    std::uniform_int_distribution<> arrivalTimeDist(0, 10);
    std::uniform_int_distribution<> priorityDist(1, 5);
    
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

// Create CORS middleware
struct CORSMiddleware {
    struct context {};
    
    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }

    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        // Nothing to do here
    }
};

int main() {
    crow::App<CORSMiddleware> app;
    
    // Handle OPTIONS requests for CORS preflight
    CROW_ROUTE(app, "/processes").methods("OPTIONS"_method)
    ([](const crow::request&){
        crow::response res;
        res.code = 204;
        return res;
    });
    
    // Route to get processes
    CROW_ROUTE(app, "/processes")
    ([]{
        auto processes = generateRandomProcesses(10); // Generate 10 random processes
        
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
    
    app.port(8080).run();
    
    return 0;
}