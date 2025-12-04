#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <omp.h>

// Función a integrar: e^(-(x1^2 + x2^2 + ...))
double func(const std::vector<double>& punto) {
    double suma = 0.0;
    for (size_t i = 0; i < punto.size(); ++i) {
        double c = punto[i];
        suma += c * c;
    }
    return std::exp(-suma);
}

int main(int argc, char* argv[]) {

    if (argc < 9) {
        std::cerr << "Usage: " << argv[0]
                  << " --li [lim inf] --ls [lim sup] --d [dim] --n [N] [--scaling]"
                  << std::endl;
        return 1;
    }

    // Parámetros desde la línea de comando
    double lim_inf = std::atof(argv[2]);
    double lim_sup = std::atof(argv[4]);
    int dimensiones = std::atoi(argv[6]);
    int N = std::atoi(argv[8]);

    bool modo_scaling = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--scaling") {
            modo_scaling = true;
        }
    }

    // ================================
    //   Inicialización paralela RNG
    // ================================
    // Generamos semillas distintas para cada hilo de forma segura
    int max_threads = omp_get_max_threads();
    std::vector<unsigned int> seeds(max_threads);

    std::random_device rd;
    for (int i = 0; i < max_threads; ++i) {
        // Combinamos random_device con un offset fijo para evitar colisiones triviales
        seeds[i] = rd() ^ static_cast<unsigned int>(12345u + 101u * i);
    }

    double suma_final = 0.0;
    double suma_final2 = 0.0;

    double time_1 = omp_get_wtime();
    int num_procs = 1;

    #pragma omp parallel reduction(+:suma_final, suma_final2)
    {
        int tid = omp_get_thread_num();

        // A cada hilo le toca su propia semilla independiente
        std::mt19937 generador(seeds[tid]);
        std::uniform_real_distribution<double> dist(lim_inf, lim_sup);

        std::vector<double> punto(dimensiones);

        // Obtenemos num_procs de forma segura (sin race condition)
        #pragma omp single
        {
            num_procs = omp_get_num_threads();
        }

        #pragma omp for
        for (int i = 0; i < N; i++) {
            for (int d = 0; d < dimensiones; d++) {
                punto[d] = dist(generador);
            }
            double val = func(punto);
            suma_final += val;
            suma_final2 += val * val;
        }
    }

    double time_2 = omp_get_wtime();

    double promedio = suma_final / N;
    double promedio_cuadrado = suma_final2 / N;
    double varianza = promedio_cuadrado - promedio * promedio;

    double volumen = 1.0;
    for (int d = 0; d < dimensiones; d++) {
        volumen *= (lim_sup - lim_inf);
    }

    double integral = promedio * volumen;
    double error = volumen * std::sqrt(varianza / N);

    if (modo_scaling) {
        // Formato esperado por scalingFourier.sh: "P T"
        std::cout << num_procs << " " << (time_2 - time_1) << std::endl;
        return 0;
    }

    std::cout << "Dimensiones: " << dimensiones << std::endl;
    std::cout << "Puntos: " << N << std::endl;
    std::cout << "Integral: " << integral << std::endl;
    std::cout << "Error: " << error << std::endl;
    std::cout << "Varianza: " << varianza << std::endl;
    std::cout << "Tiempo: " << time_2 - time_1 << " s" << std::endl;
    std::cout << "Hilos usados: " << num_procs << std::endl;

    return 0;
}
