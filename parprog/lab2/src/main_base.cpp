#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>

#define ISIZE 5000
#define JSIZE 5000

int main(int argc, char *argv[])
{
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
    for (i = 0; i < ISIZE; i++)
    {
        for (j = 0; j < JSIZE; j++)
        {
            a[i][j] = sin(2 * a[i][j]);
        }
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