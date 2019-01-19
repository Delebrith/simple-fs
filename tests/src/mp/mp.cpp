#include <string>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <signal.h>

void finishTesting(int retCode)
{
    signal(SIGQUIT, SIG_IGN);
    kill(-getpgrp(), SIGQUIT);
    exit(retCode);
}

int readPid(const char* line)
{
    char buf[64]; //for unused logs parts
    int pid;
    //date time [type][module] recv/send req/resp id to/from process pid
    sscanf(line, "%s %s %s %s %s %s %s %s %d", buf, buf, buf, buf, buf, buf, buf, buf, &pid);

    return pid;
}

void logsParser(int reqInterruptedExecutors)
{
    std::vector<int> startedRequests;

    int interruptedExecutors = 0;

    while(interruptedExecutors < reqInterruptedExecutors)
    {
        std::string line;
        std::getline(std::cin, line);

        char buf[64]; //for unused logs parts
        char msg[64];

        //date time [type][module] first_msg_word
        sscanf(line.c_str(), "%s %s %s %s", buf, buf, buf, msg);

        if (strcmp(msg, "Recived") == 0) //sic
        {
            int pid = readPid(line.c_str());
            startedRequests.push_back(pid);
        }
        if (strcmp(msg, "Sending") == 0)
        {
            int pid = readPid(line.c_str());

            if (startedRequests.size() == 0)
                finishTesting(-2);

            if (*(startedRequests.end() - 1) == pid)
            {
                startedRequests.pop_back();
            }
            else
            {
                ++interruptedExecutors;
                
                std::vector<int>::iterator pidLocation = startedRequests.begin();
                while (pidLocation != startedRequests.end() && *pidLocation != pid)
                    ++pidLocation;

                if (pidLocation == startedRequests.end())
                    finishTesting(-2);

                startedRequests.erase(pidLocation);
            }
        }
    }
}

int main()
{
    logsParser(1);

    return 0;
}