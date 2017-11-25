#include <pthread.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <utime.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
//#include <iostream>      //for debug

using namespace std;

struct data {
    vector<string> queue;//queue for files, that we need copy
    string FROM, TO;            //source and destination folder
    pthread_mutex_t mutex;      //mutex for several threads
};

bool copy_file(string& relative_path, const string& FROM, const string& TO){
    string from = FROM + relative_path;
    string to = TO + relative_path;
    struct stat in_stat;
    if(stat(from.c_str(), &in_stat) < 0){
        string temp = "stat file " + from;
        perror(temp.c_str());
        return false;
    }
    int in = open(from.c_str(), O_RDONLY);
    if(in < 0){
        string temp = "can't open file " + from;
        perror(temp.c_str());
        return  false;
    }
    struct stat last;
    if(stat(to.c_str(), &last) == 0){
        struct stat to_old;
        string new_path = to + ".old";
        //if dir with name.old already exist
        if(stat(new_path.c_str(), &to_old) == 0){
            if(S_ISDIR(to_old.st_mode)) {
                if(rmdir(new_path.c_str()) != 0){
                    string temp = "file, rmdir " + new_path + " failed";
                    perror(temp.c_str());
                    close(in);
                    return false;
                }
            } else {
                    if(unlink(new_path.c_str()) != 0){
                        string temp = "unlink " + new_path + " failed";
                        perror(temp.c_str());
                        return false;
                    }
                }
        }
            if (rename(to.c_str(), new_path.c_str()) != 0) {
                string temp = "rename " + to + " failed";
                perror(temp.c_str());
                close(in);
                return false;
            }

    }
    int out = open(to.c_str(), O_WRONLY | O_CREAT, 0777);
    if(out < 0){
        close(in);
        string temp = " can't write to " + to;
        perror(temp.c_str());
        close(in);
        return false;
    }
    char buf[4096];
    int rd;
    while ( (rd = read(in, buf, sizeof buf)) > 0) {
        write(out, &buf, rd);
    }
    if(chown(to.c_str(), in_stat.st_uid, in_stat.st_gid) < 0){
        string temp = to + " failed to change uid or gid";
        perror(temp.c_str());
        close(out);
        close(in);
        return false;
    }
    struct utimbuf time_buf;
    time_buf.actime = in_stat.st_atime;
    time_buf.modtime = in_stat.st_mtime;
    if(utime(to.c_str(), &time_buf) < 0){
        string temp = to + " can't change time's characteristics";
        perror(temp.c_str());
        close(out);
        close(in);
        return false;
    }
    if(chmod(to.c_str(), in_stat.st_mode) < 0){
        string temp = to + " failed to change mode";
        perror(temp.c_str());
        close(out);
        close(in);
        return false;
    }
    close(out);
    close(in);
    return true;
}

bool make_dir(const string& relative_path,const string& FROM,const string& TO) {
    struct stat buf;
    struct stat to_stat;
    string to = TO + relative_path;
    string from = FROM + relative_path;
    if (stat(from.c_str(), &buf) != 0) {
        string temp = "stat dir from " + from;
        perror(temp.c_str());
        return false;
    }
    if(access(to.c_str(), F_OK) == 0){
        if (stat(to.c_str(), &to_stat) < 0) {
            string temp = "stat file to " + to;
            perror(temp.c_str());
            return false;
        }
        if(S_ISDIR(to_stat.st_mode)){
            return true;
        }
        //if file with name.old already exist
        struct stat to_old;
        string new_path = to + ".old";
        if(stat(new_path.c_str(), &to_old) == 0){
            if(S_ISDIR(to_old.st_mode)) {
                if(rmdir(new_path.c_str()) != 0){
                    string temp = "rmdir " + new_path + " failed";
                    perror(temp.c_str());
                    return false;
                }
            } else {
                if(unlink(new_path.c_str()) != 0){
                    string temp = "unlink " + new_path + " failed";
                    perror(temp.c_str());
                    return false;
                }
            }
        }
        if (!S_ISDIR(to_stat.st_mode)) {
                string new_path = to + ".old";
                if (rename(to.c_str(), new_path.c_str()) != 0) {
                    string temp = "rename " + to + " failed";
                    perror(temp.c_str());
                    return false;
                }
        }
    }
    if (mkdir(to.c_str(), 0777) != 0) {
        string temp = "mkdir " + to;
        perror(temp.c_str());
        return false;
    }
    return true;
}

bool DirMetaData(const string& relative_path,const string& FROM,const string& TO){
    struct stat buf;
    struct stat to_stat;
    string to = TO + relative_path;
    string from = FROM + relative_path;
    if (stat(from.c_str(), &buf) != 0) {
        string temp = "stat dir from " + from;
        perror(temp.c_str());
        return false;
    }
    if (stat(to.c_str(), &to_stat) < 0) {
        string temp = "stat file to " + to;
        perror(temp.c_str());
        return false;
    }
    if(chown(to.c_str(), buf.st_uid, buf.st_gid) < 0){
        string temp = to + " failed to change uid or gid";
        perror(temp.c_str());
        return false;
    }
    struct utimbuf time_buf;
    time_buf.actime = buf.st_atime;
    time_buf.modtime = buf.st_mtime;
    if(utime(to.c_str(), &time_buf) < 0){
        string temp = to + " can't change time's characteristics";
        perror(temp.c_str());
        return false;
    }
    if(chmod(to.c_str(), buf.st_mode) < 0){
        string temp = to + " failed to change mode";
        perror(temp.c_str());
        return false;
    }
}



void Make_queue(vector<string>& queue, string& relative_path,const string& FROM,const string& TO, vector<string>& dir_queue){
    string path = FROM + "/" + relative_path;
    struct stat buf;
    if(stat(path.c_str(), &buf) != 0){
        string temp = "make queue stat " + path;
        perror(temp.c_str());
        return;
    }
    if(!S_ISDIR(buf.st_mode)){
        queue.push_back(relative_path);
        return;
    }
    DIR* direct = opendir(path.c_str());
    for(struct dirent* de = readdir(direct); de != NULL; de = readdir(direct)){
        if(strcmp(de->d_name, ".") == 0 || strcmp(de ->d_name, "..") == 0)
            continue;
        string relative_fname = relative_path + "/";
        relative_fname += de->d_name;
        string fname = FROM + "/";
        fname += relative_fname;
        struct stat buf;
        if(stat(fname.c_str(), &buf) == 0){
            if(S_ISDIR(buf.st_mode)){
                dir_queue.push_back(relative_fname);
                Make_queue(queue, relative_fname, FROM, TO, dir_queue);
                continue;
            } else if(S_ISREG(buf.st_mode)) {
                queue.push_back(relative_fname);
            }
            else {
                string temp = fname + " stat";
                perror(temp.c_str());
                return;
            }
        }

    }
    closedir(direct);
}

void* copy(void* arg){
    while(true) {
        data* a = (data*) arg;
        pthread_mutex_lock(&a->mutex);
        if (a->queue.size() < 1) {
            pthread_mutex_unlock(&a->mutex);
            pthread_exit(NULL);
        }
        string relative_path = (a->queue)[a->queue.size() - 1];
        a->queue.resize(a->queue.size() - 1);
        pthread_mutex_unlock(&a->mutex);
        copy_file(relative_path, a->FROM, a->TO);
    }
}


int main(int argc, char** argv) {
    if(argc != 4){
        printf("please enter valid parameters\n");
        exit(1);
    }
    if(argv[1][0] != '-' || argv[1][1] != 't'){
        printf("invalid flag");
        exit(1);
    }
    const int threads = atoi(argv[1]+2) > 1 ? atoi(argv[1]+2) : 1;
    string source = argv[2];
    string destination = argv[3];
    //unit test for parse parameters
    //cout << threads <<"\t" << FROM << "\t" << TO << endl;
    string temp;
    vector<string> dir_queue;
    vector<string> queue;
    dir_queue.push_back(temp);
    Make_queue(queue, temp, source, destination, dir_queue);

/*    //unit test for making queue
    for(int i = 0; i < queue.size(); i++){
        cout << FROM + queue[i] << endl;
    }*/

//        unit test for making dir_queue
//    for(int i = 0; i < dir_queue.size(); i++){
//        cout << FROM + dir_queue[i] << endl;
//    }

    for(int i = 0; i < dir_queue.size(); i++){
        make_dir(dir_queue[i], source, destination);
    }

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    vector<pthread_t> thrds(threads);
    data args;
    {
        args.FROM = source;
        args.TO = destination;
        args.mutex = mutex;
        args.queue = queue;
    }
    for(int i = 0; i < threads; i++){
        if(pthread_create(&thrds[i], NULL, copy, &args) != 0){
            perror("thread create");
            exit(3);
        }
    }
    for(int i= 0; i < threads; i++){
        pthread_join(thrds[i], NULL);
    }

    for(int i = 0; i < dir_queue.size(); i++){
        DirMetaData(dir_queue[i], source, destination);
    }

    return 0;
}

