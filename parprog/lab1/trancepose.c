#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define get_time(a) ((double)a.tv_sec+((double)a.tv_nsec)*1e-9)
#define time_dif(a,b) (get_time(b)-get_time(a))

typedef uint32_t uint;

void dot_T(uint* A, uint* B, uint* F, uint* D, size_t size){
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            F[i*size+j] = B[j*size+i];
        }
    }

    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            uint64_t sum = 0;
            for(int k = 0; k < size; k++){
                sum += (uint64_t)A[i*size+k]*F[j*size+k];
            }
            D[i*size+j] = (uint)sum;
        }
    }
}


int main(int argc, char** argv){
    
    static struct timespec ts[2];

    if (argc < 5) {
        fprintf(stderr, "Usage: %s <A.dat> <B.dat> <output.dat> <logfile>\n", argv[0]);
        return 1;
    }

    int size_a = 0;
    int size_b = 0;
    int len = 0;

    FILE* file_A = fopen(argv[1], "r");
    if (!file_A) {
        perror("Failed to open A matrix file");
        return 1;
    }
    if (fscanf(file_A, "%d", &size_a) != 1) {
        fprintf(stderr, "Failed to read size from %s\n", argv[1]);
        fclose(file_A);
        return 1;
    }

    FILE* file_B = fopen(argv[2], "r");
    if (!file_B) {
        perror("Failed to open B matrix file");
        fclose(file_A);
        return 1;
    }
    if (fscanf(file_B, "%d", &size_b) != 1) {
        fprintf(stderr, "Failed to read size from %s\n", argv[2]);
        fclose(file_A);
        fclose(file_B);
        return 1;
    }

    if (size_a != size_b) {
        fprintf(stderr, "Matrix size mismatch: A=%d B=%d\n", size_a, size_b);
        fclose(file_A);
        fclose(file_B);
        return 1;
    }

    int size = size_a;
    len = size*size;

    uint* A = malloc(len*sizeof(uint));
    uint* B = malloc(len*sizeof(uint));
    uint* F = malloc(len*sizeof(uint));
    uint* D = malloc(len*sizeof(uint));
    if (!A || !B || !F || !D) {
        fprintf(stderr, "Memory allocation failed for matrices of size %d\n", size);
        fclose(file_A);
        fclose(file_B);
        free(A);
        free(B);
        free(F);
        free(D);
        return 1;
    }

    for(int i = 0; i < len; i++){
        if (fscanf(file_A, "%" SCNu32, A+i) != 1 || fscanf(file_B, "%" SCNu32, B+i) != 1) {
            fprintf(stderr, "Failed to read matrix data at index %d\n", i);
            fclose(file_A);
            fclose(file_B);
            free(A);
            free(B);
            free(F);
            free(D);
            return 1;
        }
    }

    fclose(file_A);
    fclose(file_B);

    timespec_get(ts, TIME_UTC);
    dot_T(A, B, F, D, size);
    timespec_get(ts+1, TIME_UTC);

    FILE* file_D = fopen(argv[3], "w");
    if (!file_D) {
        perror("Failed to open output file");
        free(A);
        free(B);
        free(F);
        free(D);
        return 1;
    }

    fprintf(file_D, "%d\n", size);
    for(int i = 0; i < len; i++){
        fprintf(file_D, "%" PRIu32 " ", D[i]);
    }

    fclose(file_D);

    FILE* file_LOG = fopen(argv[4], "a");
    if (file_LOG) {
        double elapsed = time_dif(ts[0], ts[1]);
        fprintf(file_LOG, "TRANSPOSE,%d,%.9lf\n", size, elapsed);
        fclose(file_LOG);
    } else {
        perror("Failed to open log file");
    }

    free(A);
    free(B);
    free(F);
    free(D);

    return 0;
}
