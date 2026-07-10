#include <stdio.h>
#include <stdlib.h>

long long read_package_energy(void)
{
    FILE *fp = fopen(
        "/sys/class/powercap/intel-rapl:0/energy_uj",
        "r");

    if (!fp)
    {
        perror("Cannot open RAPL");
        exit(EXIT_FAILURE);
    }

    long long energy;

    fscanf(fp, "%lld", &energy);

    fclose(fp);

    return energy;
}