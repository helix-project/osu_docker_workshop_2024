#include <iostream>
#include <cmath>
#include "omp.h"
#include <chrono>

double calculate_pi(int num_steps){
    double pi = 0;
    double sum = 0;
    double step = 1.0/num_steps;

    int i;
    double x;

    #pragma omp parallel for reduction (+:sum) private (x)
    for ( i=0; i < num_steps; i++){
        x = (i+0.5)*step;
        // Sum over each's own sum counter
        sum += 4.0/(1.0+x*x);
    }
    pi = step * sum;
    return pi;
}

int main(int argc, char **argv){

    int num_steps = atoi(argv[1]);
    int n_repeat = 100;

    int num_threads = omp_get_max_threads();
    std::cout << "Running: " << num_steps  << " Across number of threads: " << num_threads << std::endl;
    omp_set_num_threads(num_threads);

    // Run timer
    auto start_time = std::chrono::high_resolution_clock::now();
    double pi = 0;
    for (int i = 0; i < n_repeat; i++){
        pi += calculate_pi(num_steps);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> serial_duration = end_time - start_time;

    // Print average results
    std::cout << "Calculation of Pi: " << pi  / n_repeat
              << std::endl << "Duration: " << serial_duration.count() / n_repeat << " seconds" << std::endl;
    return 0;
}