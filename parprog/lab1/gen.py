import numpy as np
import sys


MATRIX_SIZE = int(sys.argv[1])

MATRIX_A = np.random.randint(0, 256, size=(MATRIX_SIZE, MATRIX_SIZE), dtype=np.uint32)
MATRIX_B = np.random.randint(0, 256, size=(MATRIX_SIZE, MATRIX_SIZE), dtype=np.uint32)
MATRIX_C = (MATRIX_A.astype(np.uint64) @ MATRIX_B.astype(np.uint64)).astype(np.uint32)

MATRIX = MATRIX_A.reshape(-1)

with open(sys.argv[2], 'w') as f:
    f.write(f"{MATRIX_SIZE}\n")
    for i in MATRIX:
        f.write(f"{int(i)} ")
    
MATRIX = MATRIX_B.reshape(-1)

with open(sys.argv[3], 'w') as f:
    f.write(f"{MATRIX_SIZE}\n")
    for i in MATRIX:
        f.write(f"{int(i)} ")

MATRIX = MATRIX_C.reshape(-1)

with open(sys.argv[4], 'w') as f:
    f.write(f"{MATRIX_SIZE}\n")
    for i in MATRIX:
        f.write(f"{int(i)} ")
