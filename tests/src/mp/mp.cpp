#include <string>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "lib/src/simplefs.h"

void finishTesting(int retCode)
{
    signal(SIGQUIT, SIG_IGN);
    kill(-getpgrp(), SIGQUIT);

    if (retCode < 0)
        printf("FAILED\n");

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

        printf("%s\n", line.c_str());
    }
}

void runner()
{
    const char* filename = "/file";
    std::string wr = std::to_string(getpid());

    char rdbuf[10];

    while(true)
    {
        memset(rdbuf, 0, 10);

        int desc, ret;
        
        do
        {
            desc = simplefs::simplefs_create(filename, 0);
        } while (desc < 0);

        ret = simplefs::simplefs_close(desc);
        if (ret < 0)
        {
            printf("Error while closing: %d\n", ret);
            finishTesting(-2);
        }

        do
        {
            desc = simplefs::simplefs_open(filename, O_RDWR);
        } while (desc < 0);

        ret = simplefs::simplefs_write(desc, wr.c_str(), wr.length());
        if (ret != wr.length())
        {
            printf("Error while writing: %d\n", ret);
            finishTesting(-2);
        }

        ret = simplefs::simplefs_lseek(desc, 0, SEEK_SET);
        if (ret < 0)
        {
            printf("Error while lseek: %d\n", ret);
            finishTesting(-2);
        }

        ret = simplefs::simplefs_read(desc, rdbuf, 10);
        if (ret < 0)
        {
            printf("Error while reading: %d\n", ret);
            finishTesting(-2);
        }

        ret = simplefs::simplefs_close(desc);
        if (ret < 0)
        {
            printf("Error while closing: %d\n", ret);
            finishTesting(-2);
        }

        if (wr != rdbuf)
        {
            printf("Invalid content, expected \"%s\", got \"%s\"\n", wr.c_str(), rdbuf);
            finishTesting(-2);
        }
    }
}

int main(int argc, char** argv)
{
    int minimumLogInterruptions = 0;
    int processes = 0;

    if (argc == 3)
    {
        processes = atoi(argv[1]);
        minimumLogInterruptions = atoi(argv[2]);
    }
    if (minimumLogInterruptions <= 0 || processes <= 0)
    {
        printf("Invalid arguments, usage: mp <process_n> <min_exeutor_interruptions_in_log_n>");
        finishTesting(-2);
    }

    for (int i = 0; i < processes; ++i)
    {
        if (!fork())
        {
            sleep(1);
            runner();
        }
    }
    
    logsParser(5);

    printf("OK\n");
    finishTesting(0);
}