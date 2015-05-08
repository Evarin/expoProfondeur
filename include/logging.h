#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <fstream>
using namespace std;

#define LOGFILE "logProf.log"

inline void logError (string error) {
    ofstream logstream(LOGFILE, ios_base::ate);
    logstream << "Error : " << error << "\n";
    logstream.close();
};

inline void logInfo (string info) {
    ofstream logstream(LOGFILE, ios_base::ate);
    logstream << "Info : " << info << "\n";
    cout << "Info : " << info << "\n";
    logstream.close();
};


#endif // LOGGING_H
