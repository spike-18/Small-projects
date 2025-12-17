import numpy as np
import sys


with open(sys.argv[1], 'r') as f:
    size = int(f.readline())
    arr = f.readline().split(' ')

    if '' in arr:
        arr.remove('')

for i in range(size**2):
    arr[i] = int(arr[i])

arr = np.array(arr, dtype=np.uint64)
C = arr.reshape(size, size)

with open(sys.argv[2], 'r') as f:
    size = int(f.readline())
    arr = f.readline().split(' ')

    if '' in arr:
        arr.remove('')

for i in range(size**2):
    arr[i] = int(arr[i])

arr = np.array(arr, dtype=np.uint64)
R = arr.reshape(size, size)

diff = np.abs(C.astype(np.int64) - R.astype(np.int64))
print(f"MEAN_ERROR {np.mean(diff)}")
