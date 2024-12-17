#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include "log.h"
using namespace std;
void logMessage(const string &message)
{
    ofstream logFile("log.txt", ios::app); // Mở file với chế độ append
    if (logFile.is_open())
    {
        // Ghi thời gian vào log
        time_t now = time(0);
        char *dt = ctime(&now);
        dt[strlen(dt) - 1] = '\0';
        logFile << "[" << dt << "] " << message << endl;
        logFile.close();
    }
    else
    {
        cerr << "Unable to open log file" << endl;
    }
}