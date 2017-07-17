#include <stdlib.h>
#include <istream>
#include <queue>
#include <vector>
#include "Process.cpp"
#include <string>
#include <tuple>
#include <algorithm>

typedef std::tuple<int, std::string, int> event;

struct eventComparator
{
     bool operator()(event a, event b)
     {
        return std::get<2>(a) > std::get<2>(b);
     }
};

class Computer
{
    private:
        int ncores;
        int totalCores;
        int slice;
        int time;
        bool diskBusy;
        std::vector<Process> processList;
        std::priority_queue<event, std::vector<event>, eventComparator> eventHeap;
        std::queue<int> diskQueue;
        std::queue<int> readyQueue;
        void arrivalRoutine(event ev);
        void coreReqRoutine(event ev);
        void coreEndRoutine(event ev);
        void userReqRoutine(event ev);
        void userEndRoutine(event ev);
        void diskReqRoutine(event ev);
        void diskEndRoutine(event ev);
        void execute(event ev);

    public:
        Computer();
        void run();
};

Computer::Computer()
{
    ncores = 0;
    totalCores = 0;
    slice = 0;
    time = 0;
    diskBusy = false;
    std::string input;
    int processCount = 0;
    while(std::getline(std::cin, input))
    {

        std::string token = input.substr(0, input.find(" "));
        std::string temp = input.substr(input.find(" "));
        int num = std::stoi(temp);

        if(!token.compare("NCORES"))
        {
            ncores = num;
            totalCores = num;
        }

        else if(!token.compare("SLICE"))
            slice = num;

        else if(!token.compare("NEW"))
        {
            Process p(processCount, num);
            processList.push_back(p);
            event ac = std::make_tuple(processCount, "ARRIVAL", num);
            eventHeap.push(ac);
            ++processCount;
        }
        else if(!token.compare("CORE"))
        {
            while(num > slice)
            {
                std::pair<std::string, int> cmd(token, slice);
                processList.back().addCmd(cmd);
                num -= slice;
            }
            std::pair<std::string, int> cmd(token, num);
            processList.back().addCmd(cmd);
        }
        else if(!token.compare("DISK") || !token.compare("DISPLAY"))
        {
            std::pair<std::string, int> cmd(token, num);
            processList.back().addCmd(cmd);
        }
    }
}

void Computer::run()
{
    while(!eventHeap.empty())
    {
        event e = eventHeap.top();
        std::string request = std::get<1>(e);
        eventHeap.pop();
        if(!request.compare("ARRIVAL"))
            arrivalRoutine(e);
        else if(!request.compare("USER_REQ"))
            userReqRoutine(e);
        else if(!request.compare("USER_END"))
            userEndRoutine(e);
        else if(!request.compare("DISK_REQ"))
            diskReqRoutine(e);
        else if(!request.compare("DISK_END"))
            diskEndRoutine(e);
        else if(!request.compare("CORE_REQ"))
            coreReqRoutine(e);
        else if(!request.compare("CORE_END"))
            coreEndRoutine(e);
    }
}

void Computer::arrivalRoutine(event ev)
{
    int process = std::get<0>(ev);
    std::string req = std::get<1>(ev);
    time = std::get<2>(ev);
    eventHeap.push(std::make_tuple(process, "CORE_REQ", time));
}

void Computer::coreReqRoutine(event ev)
{

    int process = std::get<0>(ev);
    std::string req = std::get<1>(ev);
    if(ncores > 0)
    {
        int timeEnd = time+std::get<1>(processList.at(process).getCmdList().front());
        eventHeap.push(std::make_tuple(process, "CORE_END", timeEnd));
        --ncores;
         processList.at(process).setStatus("RUNNING");
    }
    else
    {
        readyQueue.push(process);
        processList.at(process).setStatus("READY");
    }
}

void Computer::coreEndRoutine(event ev)
{
    int process = std::get<0>(ev);
    time = std::get<2>(ev);
    ++ncores;
    processList.at(process).popCmd();
    if(!readyQueue.empty())
    {
        process = readyQueue.front();
        readyQueue.pop();
        --ncores;
        int timeEnd = time+std::get<1>(processList.at(process).getCmdList().front());
        eventHeap.push(std::make_tuple(process, "CORE_END", timeEnd));
        processList.at(process).setStatus("RUNNING");
    }
    execute(ev);
}

void Computer::userReqRoutine(event ev)
{
    int process = std::get<0>(ev);
    int timeEnd = time+std::get<1>(processList.at(process).getCmdList().front());
    eventHeap.push(std::make_tuple(process, "USER_END", timeEnd));
    processList.at(process).setStatus("BLOCKED");
}

void Computer::userEndRoutine(event ev)
{
    int process = std::get<0>(ev);
    time = std::get<2>(ev);
    processList.at(process).popCmd();
    execute(ev);
}

void Computer::diskReqRoutine(event ev)
{
    int process = std::get<0>(ev);
    if(!diskBusy)
    {
        diskBusy = true;
        int timeEnd = time+std::get<1>(processList.at(process).getCmdList().front());
        eventHeap.push(std::make_tuple(process, "DISK_END", timeEnd));
        processList.at(process).setStatus("BLOCKED");
    }
    else
    {
        diskQueue.push(process);
    }
}

void Computer::diskEndRoutine(event ev)
{

    int process = std::get<0>(ev);
    time = std::get<2>(ev);
    processList.at(process).popCmd();
    diskBusy = false;
    if(!diskQueue.empty())
    {
        eventHeap.push(std::make_tuple(diskQueue.front(), "DISK_REQ", time));
        diskQueue.pop();
    }
    execute(ev);
}

void Computer::execute(event ev)
{
    int i = std::get<0>(ev);
    if(processList.at(i).getCmdList().empty())
    {
        processList.at(i).setStatus("TERMINATED");
        std::cout << "Total Time: " << time << std::endl;
        std::cout << "Busy Cores: " << totalCores-ncores << std::endl;
        if(diskBusy)
            std::cout << "Disk Busy" << std::endl;
        else
            std::cout << "Disk Idle" << std::endl;

        std::queue<int> ready = readyQueue;
        std::queue<int> disk = diskQueue;

        std::cout << "Ready Queue: " << std::endl;
        if(ready.empty())
            std::cout << "empty" << std::endl;
        else
        {
            while(!ready.empty())
            {
               std::cout << "Process " << ready.front() << std::endl;
                ready.pop();
            }
        }
        std::cout << "Disk Queue: " << std::endl;
        if(disk.empty())
            std::cout << "empty" << std::endl;
        else
        {
            while(!disk.empty())
            {
                std::cout << "Process " << disk.front() << std::endl;
                disk.pop();
            }
        }
        for(int j = 0; j < processList.size(); ++j)
        {
            if(processList.at(j).getStatus().compare("END"))
            {
                std::cout << "Process " << processList.at(j).getPID() << ": ";
                std::cout << "started at " << processList.at(j).getStart() << " ms, ";
                std::cout << "and is " << processList.at(j).getStatus() << std::endl;
            }
        }
        processList.at(i).setStatus("END");
    }
    else
    {
        std::string req = std::get<0>(processList.at(i).getCmdList().front());
        if(!req.compare("CORE"))
        {
            eventHeap.push(std::make_tuple(i, "CORE_REQ", time));
        }
        else if(!req.compare("DISK"))
        {
            eventHeap.push(std::make_tuple(i, "DISK_REQ", time));
        }
        else if(!req.compare("DISPLAY"))
        {
            eventHeap.push(std::make_tuple(i, "USER_REQ", time));
        }
    }
}

int main()
{
    Computer c;
    c.run();
    return 0;
}
