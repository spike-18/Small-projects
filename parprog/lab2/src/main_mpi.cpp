#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include <mpi.h>

#define ISIZE 5000
#define JSIZE 5000

int main(int argc, char *argv[])
{
    int commsize, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

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

    size_t to_count = (ISIZE + commsize - 1) / commsize;

    // std::cout << rank << " " << to_count << " " << rank * to_count << std::endl;

    if (rank == 0)
    {
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < to_count; i++)
        {
            for (size_t j = 0; j < JSIZE; j++)
            {
                a[i][j] = sin(2 * a[i][j]);
            }
        }

        for (int i = 1; i < commsize - 1; i++)
        {
            MPI_Recv(a[i * to_count], to_count * JSIZE, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        MPI_Recv(a[(commsize - 1) * to_count], ISIZE * JSIZE - (to_count * JSIZE * (commsize - 1)), MPI_DOUBLE, commsize - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << duration.count() / 1000000 << "." << std::setfill('0') << std::setw(6) << duration.count() % 1000000 << std::endl;
    }
    else if (rank < commsize - 1)
    {
        for (size_t i = 0; i < to_count; i++)
        {
            // std::cout << rank << " " << i + rank * to_count << std::endl;
            for (size_t j = 0; j < JSIZE; j++)
            {
                a[i + rank * to_count][j] = sin(2 * a[i + rank * to_count][j]);
            }
        }

        MPI_Send(a[rank * to_count], to_count * JSIZE, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        for (size_t i = 0; i + rank * to_count < ISIZE; i++)
        {
            // std::cout << rank << " " << i + rank * to_count << std::endl;
            for (size_t j = 0; j < JSIZE; j++)
            {
                a[i + rank * to_count][j] = sin(2 * a[i + rank * to_count][j]);
            }
        }

        MPI_Send(a[rank * to_count], ISIZE * JSIZE - (to_count * JSIZE * (commsize - 1)), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0)
    {
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
    }

    MPI_Finalize();

    delete[] a[0];
    delete[] a;
}