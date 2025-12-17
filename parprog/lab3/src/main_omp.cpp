#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <omp.h>

struct CyclicReductionSolver {
    std::vector<double*> a_levels; // lower diagonal
    std::vector<double*> b_levels; // main 
    std::vector<double*> c_levels; // upper diagonal
    std::vector<double*> d_levels; // rhs
    std::vector<double*> x_levels; // solution
    std::vector<int> sizes;        // size of the system at each level
    int max_levels;

    CyclicReductionSolver(int N) {
        int current_size = N;
        max_levels = 0;
        
        // pre-calculate sizes and allocate memory
        // stop when size is small enough and solve directly
        while (current_size > 1) {
            sizes.push_back(current_size);
            
            a_levels.push_back(new double[current_size]);
            b_levels.push_back(new double[current_size]);
            c_levels.push_back(new double[current_size]);
            d_levels.push_back(new double[current_size]);
            x_levels.push_back(new double[current_size]);
            
            current_size = (current_size + 1) / 2;
            max_levels++;
        }
        
        // Allocate Base Case (size 1)
        sizes.push_back(current_size);
        a_levels.push_back(new double[current_size]);
        b_levels.push_back(new double[current_size]);
        c_levels.push_back(new double[current_size]);
        d_levels.push_back(new double[current_size]);
        x_levels.push_back(new double[current_size]);
        max_levels++;
    }

    ~CyclicReductionSolver() {
        for (auto p : a_levels) delete[] p;
        for (auto p : b_levels) delete[] p;
        for (auto p : c_levels) delete[] p;
        for (auto p : d_levels) delete[] p;
        for (auto p : x_levels) delete[] p;
    }

    void solve(const double* a_in, const double* b_in, const double* c_in, const double* d_in, double* result, int N) {
        
        // initialize level 0
        #pragma omp parallel for
        for(int i=0; i<N; ++i) {
            a_levels[0][i] = (i > 0) ? a_in[i-1] : 0.0; 
            b_levels[0][i] = b_in[i];
            c_levels[0][i] = (i < N-1) ? c_in[i] : 0.0;
            d_levels[0][i] = d_in[i];
        }

        // forward reduction
        for (int k = 0; k < max_levels - 1; ++k) {
            int current_n = sizes[k];
            int next_n = sizes[k+1];
            
            const double* a = a_levels[k];
            const double* b = b_levels[k];
            const double* c = c_levels[k];
            const double* d = d_levels[k];
            
            double* na = a_levels[k+1];
            double* nb = b_levels[k+1];
            double* nc = c_levels[k+1];
            double* nd = d_levels[k+1];

            #pragma omp parallel for
            for (int i = 0; i < next_n; ++i) {
                int r = 2 * i;      
                int r_prev = r - 1; 
                int r_next = r + 1;

                double alpha = 0.0; 
                double gamma = 0.0; 

                if (r_prev >= 0) {
                    alpha = -a[r] / b[r_prev];
                }
                if (r_next < current_n) {
                    gamma = -c[r] / b[r_next];
                }

                nb[i] = b[r] 
                        + (r_prev >= 0 ? alpha * c[r_prev] : 0.0)
                        + (r_next < current_n ? gamma * a[r_next] : 0.0);
                
                nd[i] = d[r] 
                        + (r_prev >= 0 ? alpha * d[r_prev] : 0.0)
                        + (r_next < current_n ? gamma * d[r_next] : 0.0);

                na[i] = (r_prev >= 0) ? alpha * a[r_prev] : 0.0;
                
                nc[i] = (r_next < current_n) ? gamma * c[r_next] : 0.0;
            }
        }

        // At the deepest level size is 1.
        int last_lvl = max_levels - 1;
        x_levels[last_lvl][0] = d_levels[last_lvl][0] / b_levels[last_lvl][0];

        // Backward substitution
        for (int k = max_levels - 2; k >= 0; --k) {
            int current_n = sizes[k];
            
            const double* a = a_levels[k];
            const double* b = b_levels[k];
            const double* c = c_levels[k];
            const double* d = d_levels[k];
            double* x = x_levels[k];
            const double* x_next = x_levels[k+1];

            // Retrieve solutions for the even indices
            #pragma omp parallel for
            for(int i=0; i < sizes[k+1]; ++i) {
                x[2*i] = x_next[i];
            }

            // Solve for the odd indices
            #pragma omp parallel for
            for(int i=0; i < current_n / 2; ++i) {
                 int r = 2 * i + 1;
                 
                 if (r < current_n) {
                     double rhs_val = d[r];
                     
                     if (r - 1 >= 0) rhs_val -= a[r] * x[r - 1];
                     
                     if (r + 1 < current_n) rhs_val -= c[r] * x[r + 1];
                     
                     x[r] = rhs_val / b[r];
                 }
            }
        }

        // Copy result back
        #pragma omp parallel for
        for(int i=0; i<N; ++i) result[i] = x_levels[0][i];
    }
};

double f(double y){ return std::exp(y); }
double df(double y){ return std::exp(y); }
double F(double y_0, double y_1, double y_2, double inv_h2){
    return (f(y_0) + 10 * f(y_1) + f(y_2)) / 12 - (y_0 - 2 * y_1 + y_2) * inv_h2;
}

int main(int argc, char *argv[]){
    if (argc != 3) { 
        std::cout << "Usage: " << argv[0] << " b total_points" << std::endl; 
        return 1; 
    }

    const double left_boundary = 1.0;
    const double right_boundary = std::stod(argv[1]);
    const ssize_t total_points = static_cast<ssize_t>(std::stoll(argv[2]));

    if (total_points < 3) return 1;

    const double h = 1.0 / (total_points - 1);
    const double inv_h2 = 1.0 / (h * h);
    double eps = 1e-7;
    const ssize_t N = total_points - 2; 

    omp_set_num_threads(4);

    double *diag_0 = new double[N];
    double *diag_1 = new double[N];
    double *diag_2 = new double[N];
    double *rhs = new double[N];
    double *y = new double[N];
    double *res = new double[N];

    CyclicReductionSolver solver(N);

    // Initial guess is linear
    #pragma omp parallel for
    for (ssize_t i = 0; i < N; i++) {
        double t = static_cast<double>(i + 1) / static_cast<double>(N + 1);
        y[i] = left_boundary + (right_boundary - left_boundary) * t;
    }

    auto start = std::chrono::high_resolution_clock::now();
    double max_err = eps * 2;
    ssize_t iteration = 0;

    while (max_err > eps) {
        
        // Parallel matrix assembly
        #pragma omp parallel for
        for (ssize_t i = 0; i < N; i++) {
            double df_val = df(y[i]);
            diag_1[i] = 10 * df_val / 12 + 2 * inv_h2;

            if(i > 0) diag_0[i-1] = df(y[i-1]) / 12 - inv_h2; 
            if(i < N-1) diag_2[i] = df(y[i+1]) / 12 - inv_h2;
        }

        #pragma omp parallel for
        for (ssize_t i = 1; i < N - 1; i++) {
            rhs[i] = -F(y[i - 1], y[i], y[i + 1], inv_h2);
        }
        
        // Boundary RHS
        if (N == 1) {
            rhs[0] = -F(left_boundary, y[0], right_boundary, inv_h2);
        } else {
            rhs[0] = -F(left_boundary, y[0], y[1], inv_h2);
            rhs[N - 1] = -F(y[N - 2], y[N - 1], right_boundary, inv_h2);
        }

        // Solve
        solver.solve(diag_0, diag_1, diag_2, rhs, res, N);

        max_err = 0.0;
        #pragma omp parallel for reduction(max: max_err)
        for (ssize_t i = 0; i < N; i++) {
            y[i] += res[i];
            if(std::abs(res[i]) > max_err) max_err = std::abs(res[i]);
        }

        iteration++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Iterations: " << iteration << std::endl;
    std::cout << duration.count() / 1000000 << "." << std::setfill('0') << std::setw(6) << duration.count() % 1000000 << std::endl;
    
    std::cerr << 0.0 << ", " << left_boundary << "\n";
    for (ssize_t i = 0; i < N; i++)
    {
        double x = h * static_cast<double>(i + 1);
        std::cerr << x << ", " << y[i] << "\n";
    }
    std::cerr << 1.0 << ", " << right_boundary << "\n";
    std::cerr << std::endl;

    delete[] diag_0; delete[] diag_1; delete[] diag_2;
    delete[] rhs; delete[] y; delete[] res;
}