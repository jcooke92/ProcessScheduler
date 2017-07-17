#include <list>
#include <utility>
#include <iostream>

typedef std::pair<std::string, int> cmdPair;

class Process
{
    private:
        int pid;
        int start;
        std::list<cmdPair> cmdList;
        std::string status;

    public:
        Process(int a, int b);
        void addCmd(cmdPair cmd);
        int getPID();
        std::list<cmdPair> getCmdList();
        void setStatus(std::string stat);
        int popCmd();
        int getStart();
        std::string getStatus();
};

Process::Process(int a, int b)
{
    pid = a;
    start = b;
    status = "READY";
}

void Process::addCmd(cmdPair cmd)
{
    cmdList.push_back(cmd);
}

int Process::getPID()
{
    return pid;
}

std::list<cmdPair> Process::getCmdList()
{
    return cmdList;
}

void Process::setStatus(std::string stat)
{
    status = stat;
}

int Process::popCmd()
{
    cmdList.pop_front();

    return (cmdList.size() == 0);
}

int Process::getStart()
{
    return start;
}

std::string Process::getStatus()
{
    return status;
}
