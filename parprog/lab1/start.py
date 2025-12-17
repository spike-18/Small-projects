import numpy as np
import os
import re
def natural_sort_key(s):

    return [
        int(text) if text.isdigit() else text.lower()
        for text in re.split('([0-9]+)', s)
    ]

files = os.listdir("DATA")

A_MATRIX = []
B_MATRIX = []
D_MATRIX = []
C_MATRIX = []

for i in files:
    if 'A' in i:
        A_MATRIX.append("DATA/"+i)
        
        base_name = i[1:] 
        B_MATRIX.append("DATA/"+"B"+base_name)
        C_MATRIX.append("DATA/"+"C"+base_name)
        D_MATRIX.append("DATA/"+"D"+base_name)

A_MATRIX.sort(key=natural_sort_key)
B_MATRIX.sort(key=natural_sort_key)
C_MATRIX.sort(key=natural_sort_key)
D_MATRIX.sort(key=natural_sort_key)

print(A_MATRIX)

os.system("rm -f LOG")
for i in range(len(A_MATRIX)):
    print(f"./slow {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]}")
    os.system(f"./build/slow {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]} LOG")
    os.system(f"py check.py {C_MATRIX[i]} {D_MATRIX[i]} >> LOG")
    
    print(f"./tr {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]}")
    os.system(f"./build/trancepose {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]} LOG")
    os.system(f"py check.py {C_MATRIX[i]} {D_MATRIX[i]} >> LOG")

    print(f"./block {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]}")
    os.system(f"./build/block {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]} LOG")
    os.system(f"py check.py {C_MATRIX[i]} {D_MATRIX[i]} >> LOG")

    print(f"./strass {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]}")
    os.system(f"./build/strass {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]} LOG")
    os.system(f"py check.py {C_MATRIX[i]} {D_MATRIX[i]} >> LOG")

    print(f"./parallel {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]}")
    os.system(f"./build/parallel {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]} LOG")
    os.system(f"py check.py {C_MATRIX[i]} {D_MATRIX[i]} >> LOG")

    print(f"./simd {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]}")
    os.system(f"./build/simd {A_MATRIX[i]} {B_MATRIX[i]} {D_MATRIX[i]} LOG")
    os.system(f"py check.py {C_MATRIX[i]} {D_MATRIX[i]} >> LOG")
    