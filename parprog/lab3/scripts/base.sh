#!/bin/bash

cd $(dirname $0)/..

POINTS=${POINTS:-4000}
B_VALUES=(0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0)

for b in "${B_VALUES[@]}"; do
    echo -n "${b}, "
    bin/main_base "${b}" "${POINTS}" 2> /dev/null
done
