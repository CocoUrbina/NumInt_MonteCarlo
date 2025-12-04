#include <omp.h>
#include <iostream>

int main() {
    std::cout << "Maximo posible: " << omp_get_max_threads() << std::endl;
}
