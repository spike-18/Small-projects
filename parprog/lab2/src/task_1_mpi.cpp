#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include <mpi.h>

#define ISIZE 5000
#define JSIZE 5000

void main_loop(int i, double **a, int sub_size, int sub_rank, int main_rank)
{
    int start, end;

    if (sub_rank + 1 != sub_size)
    {
        start = ((JSIZE - 3) / sub_size) * sub_rank;
        end = ((JSIZE - 3) / sub_size) * (sub_rank + 1);
    }
    else
    {
        start = ((JSIZE - 3) / sub_size) * sub_rank;
        end = JSIZE - 3;
    }

    for (; i < ISIZE; i += 2)
    {
        for (int j = start; j < end; j++)
        {
            a[i][j] = sin(5 * a[i - 2][j + 3]);
        }

        if (sub_rank == 0)
        {
            int start_1, end_1;

            for (int j = 1; j < sub_size; j++)
            {
                if (j + 1 < sub_size)
                {
                    start_1 = ((JSIZE - 3) / sub_size) * j;
                    end_1 = ((JSIZE - 3) / sub_size) * (j + 1);
                }
                else
                {
                    start_1 = ((JSIZE - 3) / sub_size) * (sub_size - 1);
                    end_1 = JSIZE - 3;
                }

                MPI_Recv(a[i] + start_1, end_1 - start_1, MPI_DOUBLE, j * 2 + main_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
        else
        {
            MPI_Send(a[i] + start, end - start, MPI_DOUBLE, main_rank, 0, MPI_COMM_WORLD);
        }

        if (sub_rank != 0)
        {
            MPI_Send(a[i] + start, 3, MPI_DOUBLE, sub_rank * 2 + main_rank - 2, 0, MPI_COMM_WORLD);
        }
        if (sub_rank + 1 != sub_size)
        {
            MPI_Recv(a[i] + end, 3, MPI_DOUBLE, sub_rank * 2 + main_rank + 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
}

int main(int argc, char **argv)
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

    auto start = std::chrono::high_resolution_clock::now();

    if (rank % 2 == 0)
    {
        main_loop(2, a, (commsize + 1) / 2, rank / 2, 0);

        if (rank == 0)
        {

            for (int i = 3; i < ISIZE; i += 2)
            {
                MPI_Recv(a[i], JSIZE, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
    else
    {
        main_loop(3, a, commsize / 2, rank / 2, 1);

        if (rank == 1)
        {

            for (int i = 3; i < ISIZE; i += 2)
            {
                MPI_Send(a[i], JSIZE, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
            }
        }
    }

    if (rank == 0)
    {

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
    }

    MPI_Finalize();

    delete[] a[0];
    delete[] a;
}