#!/bin/bash

cd $(dirname $0)/..

echo -n "1, "
bin/task_3_base data/task_3_base
echo -n "2, "
bin/task_3_omp data/task_3_omp_2 2
echo -n "4, "
bin/task_3_omp data/task_3_omp_4 4
echo -n "6, "
bin/task_3_omp data/task_3_omp_6 6
echo -n "8, "
bin/task_3_omp data/task_3_omp_8 8
echo -n "10, "
bin/task_3_omp data/task_3_omp_10 10
echo -n "12, "
bin/task_3_omp data/task_3_omp_12 12
echo -n "14, "
bin/task_3_omp data/task_3_omp_14 14
echo -n "16, "
bin/task_3_omp data/task_3_omp_16 16

diff data/task_3_base data/task_3_omp_2
diff data/task_3_base data/task_3_omp_4
diff data/task_3_base data/task_3_omp_6
diff data/task_3_base data/task_3_omp_8
diff data/task_3_base data/task_3_omp_10
diff data/task_3_base data/task_3_omp_12
diff data/task_3_base data/task_3_omp_14
diff data/task_3_base data/task_3_omp_16
