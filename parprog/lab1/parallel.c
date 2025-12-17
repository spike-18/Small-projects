#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <omp.h>

#define get_time(a) ((double)a.tv_sec+((double)a.tv_nsec)*1e-9)
#define time_dif(a,b) (get_time(b)-get_time(a))

typedef uint32_t uint;

typedef struct TREE_BF {
    uint *M[7];
    uint *tempA[7];
    uint *tempB[7];
    struct TREE_BF** branch;
} TREE_BF;

TREE_BF* init_tree(size_t size, size_t threshold) {
    if (size <= threshold) {
        return NULL;
    }

    TREE_BF* node = malloc(sizeof(TREE_BF));
    if (!node) return NULL;

    size_t new_size = size / 2;
    size_t block_len = new_size * new_size;
    
    uint* workspace = calloc(21 * block_len, sizeof(uint));
    if (!workspace) {
        free(node);
        return NULL;
    }

    node->M[0] = workspace;
    for (int i = 1; i < 7; ++i) node->M[i] = node->M[i-1] + block_len;
    node->tempA[0] = node->M[6] + block_len;
    for (int i = 1; i < 7; ++i) node->tempA[i] = node->tempA[i-1] + block_len;
    node->tempB[0] = node->tempA[6] + block_len;
    for (int i = 1; i < 7; ++i) node->tempB[i] = node->tempB[i-1] + block_len;

    node->branch = malloc(7 * sizeof(TREE_BF*));
    if (!node->branch) {
        free(workspace);
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < 7; ++i) {
        node->branch[i] = init_tree(new_size, threshold);
    }

    return node;
}

void free_tree(TREE_BF* node) {
    if (!node) return;

    for (int i = 0; i < 7; ++i) {
        free_tree(node->branch[i]);
    }
    
    free(node->branch);
    free(node->M[0]);
    free(node);
}

void dot(uint* A, uint* F, uint* D, size_t size, size_t total_size){
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            uint64_t sum = 0;
            for(int k = 0; k < size; k++){
                sum += (uint64_t)A[i * total_size + k] * F[j * total_size + k];
            }
            D[i * size + j] = (uint)sum;
        }
    }
}

void strass(uint* A, uint* F, uint* D, size_t size, size_t total_size, TREE_BF* buffers) {
    if (size <= 256) {
        dot(A, F, D, size, total_size);
        return;
    }

    size_t new_size = size / 2;

    uint* A11 = A;
    uint* A12 = A + new_size;
    uint* A21 = A + new_size * total_size;
    uint* A22 = A + new_size * total_size + new_size;

    uint* BT11 = F;
    uint* BT12 = F + new_size;
    uint* BT21 = F + new_size * total_size;
    uint* BT22 = F + new_size * total_size + new_size;

    uint* M[7];
    uint* tA[7];
    uint* tB[7];
    for (int i = 0; i < 7; ++i) {
        M[i] = buffers->M[i];
        tA[i] = buffers->tempA[i];
        tB[i] = buffers->tempB[i];
    }

    int use_tasks = (size >= 512);

    if (use_tasks) {
        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            uint* tempA = tA[0];
            uint* tempB = tB[0];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A11[i*total_size+j] + A22[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT11[i*total_size+j] + BT22[i*total_size+j];
            strass(tempA, tempB, M[0], new_size, new_size, buffers->branch[0]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            uint* tempA = tA[1];
            uint* tempB = tB[1];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A21[i*total_size+j] + A22[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT11[i*total_size+j];
            strass(tempA, tempB, M[1], new_size, new_size, buffers->branch[1]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            uint* tempA = tA[2];
            uint* tempB = tB[2];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A11[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT21[i*total_size+j] - BT22[i*total_size+j];
            strass(tempA, tempB, M[2], new_size, new_size, buffers->branch[2]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            uint* tempA = tA[3];
            uint* tempB = tB[3];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A22[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT12[i*total_size+j] - BT11[i*total_size+j];
            strass(tempA, tempB, M[3], new_size, new_size, buffers->branch[3]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            uint* tempA = tA[4];
            uint* tempB = tB[4];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A11[i*total_size+j] + A12[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT22[i*total_size+j];
            strass(tempA, tempB, M[4], new_size, new_size, buffers->branch[4]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            uint* tempA = tA[5];
            uint* tempB = tB[5];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A21[i*total_size+j] - A11[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT11[i*total_size+j] + BT21[i*total_size+j];
            strass(tempA, tempB, M[5], new_size, new_size, buffers->branch[5]);
        }

        #pragma omp task shared(A,F,D,buffers) firstprivate(new_size,total_size) untied
        {
            uint* tempA = tA[6];
            uint* tempB = tB[6];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempA[i*new_size+j] = A12[i*total_size+j] - A22[i*total_size+j];
            for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tempB[i*new_size+j] = BT12[i*total_size+j] + BT22[i*total_size+j];
            strass(tempA, tempB, M[6], new_size, new_size, buffers->branch[6]);
        }

        #pragma omp taskwait
    } else {
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[0][i*new_size+j] = A11[i*total_size+j] + A22[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[0][i*new_size+j] = BT11[i*total_size+j] + BT22[i*total_size+j];
        strass(tA[0], tB[0], M[0], new_size, new_size, buffers->branch[0]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[1][i*new_size+j] = A21[i*total_size+j] + A22[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[1][i*new_size+j] = BT11[i*total_size+j];
        strass(tA[1], tB[1], M[1], new_size, new_size, buffers->branch[1]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[2][i*new_size+j] = A11[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[2][i*new_size+j] = BT21[i*total_size+j] - BT22[i*total_size+j];
        strass(tA[2], tB[2], M[2], new_size, new_size, buffers->branch[2]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[3][i*new_size+j] = A22[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[3][i*new_size+j] = BT12[i*total_size+j] - BT11[i*total_size+j];
        strass(tA[3], tB[3], M[3], new_size, new_size, buffers->branch[3]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[4][i*new_size+j] = A11[i*total_size+j] + A12[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[4][i*new_size+j] = BT22[i*total_size+j];
        strass(tA[4], tB[4], M[4], new_size, new_size, buffers->branch[4]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[5][i*new_size+j] = A21[i*total_size+j] - A11[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[5][i*new_size+j] = BT11[i*total_size+j] + BT21[i*total_size+j];
        strass(tA[5], tB[5], M[5], new_size, new_size, buffers->branch[5]);

        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tA[6][i*new_size+j] = A12[i*total_size+j] - A22[i*total_size+j];
        for(int i=0; i < new_size; i++) for(int j=0; j < new_size; j++) tB[6][i*new_size+j] = BT12[i*total_size+j] + BT22[i*total_size+j];
        strass(tA[6], tB[6], M[6], new_size, new_size, buffers->branch[6]);
    }

    for(int i=0; i < new_size; i++){
        for(int j=0; j < new_size; j++){
            size_t m_idx = i * new_size + j;
            D[(i)*total_size + (j)] = M[0][m_idx] + M[3][m_idx] - M[4][m_idx] + M[6][m_idx];
            D[(i)*total_size + (j + new_size)] = M[2][m_idx] + M[4][m_idx];
            D[(i + new_size)*total_size + (j)] = M[1][m_idx] + M[3][m_idx];
            D[(i + new_size)*total_size + (j + new_size)] = M[0][m_idx] - M[1][m_idx] + M[2][m_idx] + M[5][m_idx];
        }
    }
}

void block_dot(uint* A, uint* F, uint* D, size_t size, size_t total_size) {
    TREE_BF* buffer_tree_root = init_tree(size, 256);
    #pragma omp task shared(A,F,D,buffer_tree_root) firstprivate(size,total_size)
    strass(A, F, D, size, total_size, buffer_tree_root);
    #pragma omp taskwait
    free_tree(buffer_tree_root);
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
    uint* D = calloc(len, sizeof(uint));
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

    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            F[j*size+i] = B[i*size+j];
        }
    }

    omp_set_nested(1);
    omp_set_num_threads(omp_get_max_threads());

    timespec_get(ts, TIME_UTC);
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            block_dot(A, F, D, size, size);
        }
    }
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
        fprintf(file_LOG, "PARALLEL_STRASSEN_TRANSPOSE,%d,%.9lf\n", size, elapsed);
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
