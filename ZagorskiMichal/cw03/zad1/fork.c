#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>
#include <time.h>

#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 199309L

struct mz_time{
    double user;
    double sys;
    double real;
};

int loop = 0;
typedef struct mz_time mz_time_t;

void print(mz_time_t* val){
    printf("\n");
    printf(" - sys: %lf ms \n - user: %lf ms\n - real: %lf ms\n", val->sys, val->user, val->real);
}

void child_inc();

int main(int argc, char** argv){
    if(argc != 2){
        perror("Wrong number of arguments");
    }
    int N = atoi(argv[1]);
    struct rusage parent;
    struct rusage child;
    mz_time_t begin;
    clock_t begin_time = clock();
    if(getrusage(RUSAGE_SELF, &parent) == -1){
        perror(NULL);
    }

    begin.real = (double)begin_time / ((double) CLOCKS_PER_SEC / 1000.0);
    begin.user = ((double) parent.ru_utime.tv_sec ) * 1000.0
                + ((double) parent.ru_utime.tv_usec) / 1000.0;
    begin.sys = ((double) parent.ru_stime.tv_sec ) * 1000.0
                + ((double) parent.ru_stime.tv_usec) / 1000.0;
    mz_time_t s_child;
    mz_time_t children;
    children.user = 0.0;
    children.sys = 0.0;
    children.real = 0.0;
    for(int i = 0; i < N; i++){
        int child_real = 0;
        //long child_begin = clock();
        if(getrusage(RUSAGE_CHILDREN, &child) == -1){
            perror(NULL);
        }
        s_child.real = (double)child_real / ((double) CLOCKS_PER_SEC / 1000.0);
        s_child.user = ((double) child.ru_utime.tv_sec ) * 1000.0
                    + ((double) child.ru_utime.tv_usec) / 1000.0;
        s_child.sys = ((double) parent.ru_stime.tv_sec ) * 1000.0
                    + ((double) child.ru_stime.tv_usec) / 1000.0;
                // Wait until finish the process
        pid_t pid = fork();
        if(pid == -1){
            perror(NULL);
        }
        if(pid == 0){
            loop++;
            _exit((int)clock());
        }
        if (pid > 0){
            if(wait(&child_real) != pid){
                perror("Wait");
            }
            if(getrusage(RUSAGE_CHILDREN, &child) == -1){
                perror(NULL);
            }
            s_child.real = (double)child_real / ((double) CLOCKS_PER_SEC / 1000.0) - s_child.real;
            s_child.user = ((double) child.ru_utime.tv_sec ) * 1000.0
                        + ((double) child.ru_utime.tv_usec) / 1000.0 - s_child.user;
            s_child.sys = ((double) parent.ru_stime.tv_sec ) * 1000.0
                        + ((double) child.ru_stime.tv_usec) / 1000.0 - s_child.sys;
                    // Wait until finish the process
            children.real += s_child.real;
            children.user += s_child.user;
            children.sys += s_child.sys;
        }
    }
    mz_time_t end_time;
    clock_t e_time = clock();
    if(getrusage(RUSAGE_SELF, &parent) == 1){
        perror(NULL);
    }
    end_time.real = (double)e_time / ((double) CLOCKS_PER_SEC / 1000.0);
    end_time.user = ((double)parent.ru_utime.tv_sec ) * 1000.0
                + ((double)parent.ru_utime.tv_usec) / 1000.0;
    end_time.sys = ((double)parent.ru_stime.tv_sec ) * 1000.0
                + ((double)parent.ru_stime.tv_usec) / 1000.0;
    printf("Loop value: %d\n", loop);
    end_time.real = end_time.real - begin.real;
    end_time.user = end_time.user - begin.user;
    end_time.sys = end_time.sys - begin.sys;
    printf("\nFORK: %d\n", N);
    printf("--------------------------------------\n");
    print(&children);
    print(&end_time);
    return 0;
}
