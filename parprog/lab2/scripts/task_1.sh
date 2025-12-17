#!/bin/bash

cd $(dirname $0)/..

echo -n "1, "
bin/task_1_base data/task_1_base
echo -n "2, "
mpirun -np 2 bin/task_1_mpi data/task_1_mpi_2
echo -n "4, "
mpirun -np 4 bin/task_1_mpi data/task_1_mpi_4
echo -n "6, "
mpirun -np 6 bin/task_1_mpi data/task_1_mpi_6
echo -n "8, "
mpirun -np 8 bin/task_1_mpi data/task_1_mpi_8
echo -n "10, "
mpirun -np 10 bin/task_1_mpi data/task_1_mpi_10
echo -n "12, "
mpirun -np 12 bin/task_1_mpi data/task_1_mpi_12
echo -n "14, "
mpirun -np 14 bin/task_1_mpi data/task_1_mpi_14
echo -n "16, "
mpirun -np 16 bin/task_1_mpi data/task_1_mpi_16

diff data/task_1_base data/task_1_mpi_2
diff data/task_1_base data/task_1_mpi_4
diff data/task_1_base data/task_1_mpi_6
diff data/task_1_base data/task_1_mpi_8
diff data/task_1_base data/task_1_mpi_10
diff data/task_1_base data/task_1_mpi_12
diff data/task_1_base data/task_1_mpi_14
diff data/task_1_base data/task_1_mpi_16
