#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#include "rapl.h"

//Timing

double current_time()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec + ts.tv_nsec / 1e9;
}

//Main

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr,
            "Usage:\n"
            "./measure command [args...]\n");

        return 1;
    }

//First energy read

    long long start_energy =
        read_package_energy();

    double start_time =
        current_time();


    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(argv[1], &argv[1]);

        perror("execvp");

        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);

        double end_time =
        current_time();

    long long end_energy =
        read_package_energy();
    double joules =
        (end_energy - start_energy)
        / 1000000.0;

    double runtime =
        end_time - start_time;
    printf("Energy : %.6f J\n", joules);

    printf("Time   : %.6f s\n", runtime);

    FILE *csv =
        fopen("results.csv", "a");

    fprintf(csv,
        "%.6f,%.6f\n",
        joules,
        runtime);

    fclose(csv);

    return 0;
}

