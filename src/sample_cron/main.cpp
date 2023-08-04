#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

struct Task {
    int minute;
    int hour;
    std::string command;
};

std::vector<Task> readTasks(const std::string& filename) {
    std::vector<Task> tasks;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Task task;
        if (!(iss >> task.minute >> task.hour)) {
            break;
        }
        std::getline(iss, task.command);  // Read the rest of the line as command
        std::cout << "hour:" << task.hour << " minute:" << task.minute
                  << " command:" << task.command << std::endl;
        tasks.push_back(task);
    }

    return tasks;
}

// Function to execute a task
void executeTask(const std::string& command) {
    int exitCode = std::system(command.c_str());
    if (exitCode != 0) {
        std::cerr << "Task execution failed with exit code: " << exitCode << std::endl;
    }
}

int main() {
    std::string configFile = "cron_config.txt";  // Replace with your config file path
    std::vector<Task> tasks = readTasks(configFile);

    while (true) {
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
        struct std::tm* timeinfo = std::localtime(&currentTime);

        for (const Task& task : tasks) {
            if ((task.hour == timeinfo->tm_hour && task.minute == timeinfo->tm_min) ||
                (task.minute == -1 && task.hour == timeinfo->tm_hour) ||
                (task.minute == -1 && task.hour == -1)) {
                executeTask(task.command);
            }
        }

        // Sleep for one minute
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }

    return 0;
}
