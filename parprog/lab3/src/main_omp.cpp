#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <omp.h>

double get_element(double *a, ssize_t i, ssize_t size){
    if (i < 0)
        return 0;
    if (i >= size)
        return 0;
    return a[i];
}

void set_element(double *a, double value, ssize_t i, ssize_t size){
    if (i < 0)
        return;
    if (i >= size)
        return;
    a[i] = value;
}

void solve_tridiag(double *diag_0, double *diag_1, double *diag_2, double *b, double *res, ssize_t size){
    for (ssize_t i = 1; i < size; i++)
    {
        double c = diag_0[i - 1] / diag_1[i - 1];
        diag_1[i] -= diag_2[i - 1] * c;
        b[i] -= b[i - 1] * c;
    }

    for (ssize_t i = size - 2; i + 1 > 0; i--)
    {
        double c = diag_2[i] / diag_1[i + 1];
        b[i] -= b[i + 1] * c;
    }

    for (ssize_t i = 0; i < size; i++)
        res[i] = b[i] / diag_1[i];
}

void solve_reduction(double *diag_0, double *diag_1, double *diag_2, double *rhs, double *res, ssize_t size, ssize_t height)
{
    if (height == 0 || size <= 2)
    {
        solve_tridiag(diag_0, diag_1, diag_2, rhs, res, size);
        return;
    }

    double *diag_0_0, *diag_0_1, *diag_0_2, *diag_1_0, *diag_1_1, *diag_1_2, *rhs_0, *rhs_1, *res_0, *res_1;

    ssize_t size_0, size_1;

    size_1 = size / 2;
    size_0 = size - size_1;

    diag_0_0 = new double[size_0 - 1];
    diag_0_1 = new double[size_0];
    diag_0_2 = new double[size_0 - 1];
    rhs_0 = new double[size_0];
    res_0 = new double[size_0];

    diag_1_0 = new double[size_1 - 1];
    diag_1_1 = new double[size_1];
    diag_1_2 = new double[size_1 - 1];
    rhs_1 = new double[size_1];
    res_1 = new double[size_1];

    auto reduction_body = [&]() {
#pragma omp for
        for (ssize_t j = 0; j < size_0; j++)
        {
            double a, b, c, d, e, f, g, h, i, x, y, z, u, v, w, q;

            a = get_element(diag_0, j * 2 - 2, size - 1);
            b = get_element(diag_1, j * 2 - 1, size);
            c = get_element(diag_2, j * 2 - 1, size - 1);
            d = get_element(diag_0, j * 2 - 1, size - 1);
            e = get_element(diag_1, j * 2 + 0, size);
            f = get_element(diag_2, j * 2 + 0, size - 1);
            g = get_element(diag_0, j * 2 + 0, size - 1);
            h = get_element(diag_1, j * 2 + 1, size);
            i = get_element(diag_2, j * 2 + 1, size - 1);
            u = get_element(rhs, j * 2 - 1, size);
            v = get_element(rhs, j * 2 + 0, size);
            w = get_element(rhs, j * 2 + 1, size);

            y = e;
            q = v;

            if (b != 0)
            {
                x = -a * d / b;
                y -= c * d / b;
                q -= u * d / b;
            }
            else
            {
                x = 0;
            }

            if (h != 0)
            {
                z = -i * f / h;
                y -= g * f / h;
                q -= w * f / h;
            }
            else
            {
                z = 0;
            }

            set_element(diag_0_0, x, j - 1, size_0 - 1);
            set_element(diag_0_1, y, j + 0, size_0);
            set_element(diag_0_2, z, j + 0, size_0 - 1);
            set_element(rhs_0, q, j + 0, size_0);
        }

#pragma omp for
        for (ssize_t j = 0; j < size_1; j++)
        {
            double a, b, c, d, e, f, g, h, i, x, y, z, u, v, w, q;

            a = get_element(diag_0, j * 2 - 1, size - 1);
            b = get_element(diag_1, j * 2 + 0, size);
            c = get_element(diag_2, j * 2 + 0, size - 1);
            d = get_element(diag_0, j * 2 + 0, size - 1);
            e = get_element(diag_1, j * 2 + 1, size);
            f = get_element(diag_2, j * 2 + 1, size - 1);
            g = get_element(diag_0, j * 2 + 1, size - 1);
            h = get_element(diag_1, j * 2 + 2, size);
            i = get_element(diag_2, j * 2 + 2, size - 1);
            u = get_element(rhs, j * 2 + 0, size);
            v = get_element(rhs, j * 2 + 1, size);
            w = get_element(rhs, j * 2 + 2, size);

            y = e;
            q = v;

            if (b != 0)
            {
                x = -a * d / b;
                y -= c * d / b;
                q -= u * d / b;
            }
            else
            {
                x = 0;
            }

            if (h != 0)
            {
                z = -i * f / h;
                y -= g * f / h;
                q -= w * f / h;
            }
            else
            {
                z = 0;
            }

            set_element(diag_1_0, x, j - 1, size_1 - 1);
            set_element(diag_1_1, y, j + 0, size_1);
            set_element(diag_1_2, z, j + 0, size_1 - 1);
            set_element(rhs_1, q, j + 0, size_1);
        }

#pragma omp single
        {
#pragma omp task
            solve_reduction(diag_0_0, diag_0_1, diag_0_2, rhs_0, res_0, size_0, height - 1);
#pragma omp task
            solve_reduction(diag_1_0, diag_1_1, diag_1_2, rhs_1, res_1, size_1, height - 1);
#pragma omp taskwait
        }

#pragma omp for
        for (ssize_t i = 0; i < size; i += 2)
        {
            res[i] = res_0[i / 2];
        }
#pragma omp for
        for (ssize_t i = 1; i < size; i += 2)
        {
            res[i] = res_1[i / 2];
        }
    };

#pragma omp parallel
    {
        reduction_body();
    }

    delete[] diag_0_0;
    delete[] diag_0_1;
    delete[] diag_0_2;
    delete[] rhs_0;
    delete[] res_0;

    delete[] diag_1_0;
    delete[] diag_1_1;
    delete[] diag_1_2;
    delete[] rhs_1;
    delete[] res_1;
}


double f(double y){
    return std::exp(y);
}

double df(double y){
    return std::exp(y);
}

double F(double y_0, double y_1, double y_2, double h){
    return (f(y_0) + 10 * f(y_1) + f(y_2)) / 12 - (y_0 - 2 * y_1 + y_2) / (h * h);
}

int main(int argc, char *argv[]){
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " b total_points" << std::endl;
        return 1;
    }

    const double left_boundary = 1.0;
    const double right_boundary = std::stod(argv[1]);

    const ssize_t total_points = static_cast<ssize_t>(std::stoll(argv[2]));
    if (total_points < 3)
    {
        std::cerr << "total_points must be at least 3 (including boundaries)" << std::endl;
        return 1;
    }

    const double h = 1.0 / (total_points - 1);
    const double inv_h2 = 1.0 / (h * h);
    double eps = 1e-7;

    const ssize_t N = total_points - 2; // interior points

    omp_set_num_threads(4);
    const ssize_t max_height = static_cast<ssize_t>(std::log2(std::max<ssize_t>(1, N)));
    const ssize_t thread_height = static_cast<ssize_t>(std::log2(std::max(1, omp_get_max_threads())));
    const ssize_t height = std::max<ssize_t>(1, std::min(max_height, thread_height + 1)); // expose enough tasks to match threads

    auto start = std::chrono::high_resolution_clock::now();
    double *diag_0 = new double[N - 1];
    double *diag_1 = new double[N];
    double *diag_2 = new double[N - 1];
    double *rhs = new double[N];
    double *y = new double[N];

    double *res = new double[N];

    for (ssize_t i = 0; i < N; i++)
    {
        double t = static_cast<double>(i + 1) / static_cast<double>(N + 1);
        y[i] = left_boundary + (right_boundary - left_boundary) * t;
    }

    double max = eps * 2;

    while (max > eps)
    {
        #pragma omp parallel
        {
            #pragma omp for
            for (ssize_t i = 0; i < N - 1; i++)
            {
                diag_0[i] = df(y[i]) / 12 - inv_h2;
            }

            #pragma omp for
            for (ssize_t i = 0; i < N; i++)
            {
                diag_1[i] = 10 * df(y[i]) / 12 + 2 * inv_h2;
            }

            #pragma omp for
            for (ssize_t i = 0; i < N - 1; i++)
            {
                diag_2[i] = df(y[i + 1]) / 12 - inv_h2;
            }

            #pragma omp for
            for (ssize_t i = 1; i < N - 1; i++)
            {
                rhs[i] = -F(y[i - 1], y[i], y[i + 1], h);
            }
        }

        if (N == 1)
            rhs[0] = -F(left_boundary, y[0], right_boundary, h);
        else
        {
            rhs[0] = -F(left_boundary, y[0], y[1], h);
            rhs[N - 1] = -F(y[N - 2], y[N - 1], right_boundary, h);
        }

        solve_reduction(diag_0, diag_1, diag_2, rhs, res, N, height);

#pragma omp parallel for
        for (ssize_t i = 0; i < N; i++)
        {
            y[i] += res[i];
        }

        max = std::abs(res[0]);

        for (ssize_t i = 1; i < N; i++)
        {
            max = std::max(max, std::abs(res[i]));
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << duration.count() / 1000000 << "." << std::setfill('0') << std::setw(6) << duration.count() % 1000000 << std::endl;

    std::cerr << 0.0 << ", " << left_boundary << "\n";
    for (ssize_t i = 0; i < N; i++)
    {
        double x = h * static_cast<double>(i + 1);
        std::cerr << x << ", " << y[i] << "\n";
    }
    std::cerr << 1.0 << ", " << right_boundary << "\n";
    std::cerr << std::endl;
}
