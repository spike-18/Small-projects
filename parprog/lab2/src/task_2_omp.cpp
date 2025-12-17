#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include <omp.h>

#define ISIZE 5000
#define JSIZE 5000

void some_loop(int j_0, double **a)
{
    for (int i = 0; i < ISIZE - 1; i++)
    {
        for (int j = j_0; j < JSIZE; j += 6)
        {
            a[i][j] = sin(0.2 * a[i + 1][j - 6]);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        omp_set_num_threads(std::stoi(argv[2]));
    }
    
    double **a = new double *[ISIZE];
    a[0] = new double[ISIZE * JSIZE];

    for (size_t i = 1; i < ISIZE; i++)
    {
        a[i] = a[i - 1] + JSIZE;
    }

    int i, j;

    std::ofstream ff;

    for (i = 0; i < ISIZE; i++)
    {
        for (j = 0; j < JSIZE; j++)
        {
            a[i][j] = 10 * i + j;
        }
    }

    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel for
    for (int k = 6; k < 12; k++)
    {
        some_loop(k, a);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << duration.count() / 1000000 << "." << std::setfill('0') << std::setw(6) << duration.count() % 1000000 << std::endl;

    if (argc > 1)
    {
        ff.open(argv[1]);
        for (i = 0; i < ISIZE; i++)
        {
            for (j = 0; j < JSIZE; j++)
            {
                ff << a[i][j];
            }
            ff << "\n";
        }
        ff.close();
    }

    delete[] a[0];
    delete[] a;
}