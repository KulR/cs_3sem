#include <unistd.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
//#include <signal.h>
#include <sys/timeb.h>
#include <time.h>
//delete after debug
#define file_name "mycrontab"
#define mus_in_ms 1000
using namespace std;

struct Date {    //* == -1
    int hours;
    int minutes;
    int seconds;
};

void Parse_file(vector <vector<const char *>> &args, vector <Date> &schedule, ifstream &input);

void execution_in_time(vector <vector<const char *>> &args, vector <Date> &schedule);

void execution(vector <vector<const char *>> &args, int N);

struct tm *current_time() {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return timeinfo;
}


int main() {
//    time_t t = time(NULL);
//    localtime(time);
//    cout << time%86400/3600;

    vector <vector<const char *>> args;
    vector <Date> schedule;
    ifstream input(file_name);
    if (!input.is_open()) {
        cout << "file not open!" << endl;
    }
    Parse_file(args, schedule, input);
    struct stat file_buf;
    if (stat(file_name, &file_buf) == -1) {
        perror(NULL);
    }
    int parse_time = file_buf.st_mtime;
    while (true) {
        if (!input.is_open()) {
            cout << "file not open!" << endl;
        }
        if (stat(file_name, &file_buf) == -1) {
            perror(NULL);
        }
        if (parse_time != file_buf.st_mtime) {
            input.close();
            input.open(file_name);
            Parse_file(args, schedule, input);
            parse_time = file_buf.st_mtime;
        }
        execution_in_time(args, schedule);
        struct timeb time;
        ftime(&time);
        int mtime = 1000 - time.millitm;    //1000 ms in s
        usleep(mus_in_ms * (mtime + 2));    //2ms is a delay
    }
}

void Parse_file(vector <vector<const char *>> &args, vector <Date> &schedule, ifstream &input) {
    args = {};
    schedule = {};
    string temp;
    vector <vector<string>> argv;
    for (int n_str = 0; getline(input, temp); n_str++) {     //n_str - string number
        argv.push_back({});
        Date date;
        int a[3];    // a[0] - hours, a[1] - minutes, a[2] - seconds
        int begin = 0; //position of begin block
        //begin parse date
        for (int i = 0; i < 3; i++) {
            if (temp[begin + 1] == ':' || temp[begin + 1] == ' ') {
                if (temp[begin] == '*') {
                    a[i] = -1;
                } else {
                    a[i] = temp[begin] - '0';
                }
                begin += 2;
            } else {
                a[i] = 10 * (temp[begin] - '0') + temp[begin + 1] - '0';
                begin += 3;
            }
        }
        date.hours = a[0];
        date.minutes = a[1];
        date.seconds = a[2];
        schedule.push_back(date);
        // end parse date

        int position = begin; //position of read

        //parse command
        for (; position < temp.size(); position++) {
            if (temp[position] == ' ') {
                if(position == 0 || temp[position - 1] != ' '){
                    argv[n_str].push_back(temp.substr(begin, position - begin));
                    begin = position + 1;
                }
                else {
                    begin++;
                }
            }
        }
        argv[n_str].push_back(temp.substr(begin, temp.size() - begin));
        //end parse command
    }
    //convert
    for (int j = 0; j < argv.size(); j++) {
        args.push_back({});
        for (int i = 0; i < argv[j].size(); i++) {
            args[j].push_back(argv[j][i].c_str());
        }
        args[j].push_back(NULL);
    }
}


void execution_in_time(vector <vector<const char *>> &args, vector <Date> &schedule) {
//    struct tm now = *(current_time());
//
    struct timeb current;
    ftime(&current);
//    cout << "time:" << current.time % 86400 / 3600 << ":" << current.time % 3600 / 60 << ":" << current.time % 60
//         << ":"
//         << current.millitm << endl;
    for (int i = 0; i < schedule.size(); i++) {
        if (schedule[i].hours == -1 || schedule[i].hours == current.time % 86400 / 3600) {
            if (schedule[i].minutes == -1 || schedule[i].minutes == current.time % 3600 / 60) {
                if (schedule[i].seconds == -1 || schedule[i].seconds == current.time % 60) {
                    execution(args, i);
                }
            }
        }
    }
}

void execution(vector <vector<const char *>> &args, int N) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[N][0], (char **) &args[N][0]);
        perror("execution failed:");
        exit(42);
    }
}