#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>

const int mcsins = 1000000;
using namespace std;

struct block{
    bool IsBuilt = false;
    int Captured = -1;
};
/*
Captured == -1 - not captured
Captured == 0  - father captured
Captured == 1  - son captured

half == -1 - left half
half ==  0 - whole block
half ==  1 - right half
*/

void CheckHalf(string& bl, const string& ch, const int WhatHalf){
    if(WhatHalf == -1 || WhatHalf == 0){
        bl += "[";
    }
    bl += ch;
    if(WhatHalf == 0){
        bl+=ch;
    }
    if(WhatHalf == 0 || WhatHalf == 1){
        bl += "]";
    }
}

void PrintBlock(block b, int WhatHalf){
    string bl;
    if(b.IsBuilt){
        CheckHalf(bl, "_", WhatHalf);
    }
    else if(b.Captured == 0){
        CheckHalf(bl, "o", WhatHalf);
    }
    else if(b.Captured == 1){
        CheckHalf(bl, "c", WhatHalf);
    }
    else{
        bl = "    ";
    }
    printf("%s", bl.c_str());
}

void PrintWall(const vector<vector<block> >& wall){
//    system("clear");
    printf("\033c");
    for(int i = wall.size() - 1; i > -1; i--){
        if((wall.size() - 1 - i) % 2){
            for(int j = 0; j < wall[i].size(); j++){
                PrintBlock(wall[i][j], 0);
            }
            printf("\n");
        }
        else{
            PrintBlock(wall[i][0], 1);
            for(int j = 1; j < wall[i].size() - 1; j++){
                PrintBlock(wall[i][j], 0);
            }
            PrintBlock(wall[i][wall[i].size() - 1], -1);
            printf("\n");
        }
    }
}

struct Data {
    int Tmin, Tmax;
    pthread_mutex_t* mutex;
    vector<vector<block> >* wall;
    bool IsSon;
};

block* Findblock(bool IsSon, Data* a){
    for(int i = IsSon; i < (*(a->wall)).size(); i++){
        for(int j = 0; j < (*(a->wall))[i].size(); j++){
            if(!((*(a->wall))[i][j].IsBuilt) && (*(a->wall))[i][j].Captured == -1){
                if(i == 0 && (j ==0 || (*(a->wall))[0][j-1].IsBuilt)){
                    (*(a->wall))[i][j].Captured = IsSon;
                    return &(*(a->wall))[i][j];
                }
                else{
                    if(i % 2){
                        if(j == 0){
                            if((*(a->wall))[i-1][0].IsBuilt) {
                                (*(a->wall))[i][j].Captured = IsSon;
                                return &(*(a->wall))[i][j];
                            }
                        }else {
                            if(j == (*(a->wall))[i].size() - 1){
                                if((*(a->wall))[i][j-1].IsBuilt && (*(a->wall))[i-1][j-1].IsBuilt){
                                    (*(a->wall))[i][j].Captured = IsSon;
                                    return &(*(a->wall))[i][j];
                                }
                            }
                            if((*(a->wall))[i][j-1].IsBuilt && ((*(a->wall))[i-1][j-1].IsBuilt
                                                              && (*(a->wall))[i-1][j].IsBuilt)){
                                (*(a->wall))[i][j].Captured = IsSon;
                                return &(*(a->wall))[i][j];
                            }
                        }
                    } else {
                        if(j == 0){
                            if((*(a->wall))[i-1][0].IsBuilt && (*(a->wall))[i-1][1].IsBuilt) {
                                (*(a->wall))[i][j].Captured = IsSon;
                                return &(*(a->wall))[i][j];
                            }
                        } else {
                            if((*(a->wall))[i][j-1].IsBuilt
                               && ((*(a->wall))[i-1][j].IsBuilt && (*(a->wall))[i-1][j+1].IsBuilt)){
                                (*(a->wall))[i][j].Captured = IsSon;
                                return &(*(a->wall))[i][j];
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

bool CheckWall(const vector<vector<block> >& wall) {
    for (int i = 0; i < wall.size(); i++) {
        for (int j = 0; j < wall[i].size(); j++) {
            if(!wall[i][j].IsBuilt){
                return false;
            }
        }
    }
    return true;
}

void* builders(void* arg){
    Data *a = (Data*)arg;
    int time = (a->Tmax - a->Tmin) == 0 ? a->Tmax : (a->Tmin+rand()%(a->Tmax - a->Tmin));
    block* bl = NULL;
    while(true) {
        pthread_mutex_lock(a->mutex);
        bl = Findblock(a->IsSon, a);
        if(bl == NULL){
            if(CheckWall(*(a->wall))) {
                pthread_mutex_unlock(a->mutex);
                pthread_exit(NULL);
            }
        } else {
        pthread_mutex_unlock(a->mutex);
        usleep(time*mcsins);
        pthread_mutex_lock(a->mutex);
        bl->Captured = -1;
        bl->IsBuilt = true;
        pthread_mutex_unlock(a->mutex);
        }
    }
}

void* Print(void* arg){
    Data* a = (Data*) arg;
    while(true){
        pthread_mutex_lock(a->mutex);
        PrintWall(*(a->wall));
        if(CheckWall(*(a->wall))){
            pthread_mutex_unlock(a->mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(a->mutex);
        usleep(mcsins/25);
    }
}

int main() {
    unsigned int M, N;
    int Pmin, Pmax, Smin, Smax;
    printf("Please Enter parameters(N, M, Pmin, Pmax, Smin, Smax)\n");
    scanf("%u %u %i %i %i %i", &N, &M, &Pmin, &Pmax, &Smin, &Smax);
    //wall initialized
    vector<vector<block> > wall(M);
    for(int i = 0; i < wall.size(); i++) {
        if ((wall.size() - 1 - i) % 2) {
            vector<block> temp(N);
            wall[i] = temp;
        } else {
            vector<block> temp(N+1);
            wall[i] = temp;
        }
    }
    pthread_mutex_t* mutex;
    pthread_mutex_init(mutex, NULL);
    vector<pthread_t> thrds(3);
    vector<Data> args(3);
    for(int i = 0; i < 3; i++){
        args[i].mutex = mutex;
        args[i].wall = &wall;
    }
    args[0].IsSon = false;
    args[1].IsSon = true;
    args[0].Tmin = Pmin;
    args[0].Tmax = Pmax;
    args[1].Tmin = Smin;
    args[1].Tmax = Smax;
    if(pthread_create(&thrds[2], NULL, Print, &args[2]) != 0){
        perror("thread create");
        exit(3);
    }
    if(pthread_create(&thrds[0], NULL, builders, &args[0]) != 0){
        perror("thread create");
        exit(3);
    }
    if(pthread_create(&thrds[1], NULL, builders, &args[1]) != 0){
        perror("thread create");
        exit(3);
    }
    for(int i= 0; i < 3; i++){
        pthread_join(thrds[i], NULL);
    }
    printf("Make great wall again!\n");
    return 0;
}