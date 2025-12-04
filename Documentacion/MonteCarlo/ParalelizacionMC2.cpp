#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <omp.h>

// Función a integrar: e^(-(x1^2 + x2^2 + ...))
double func(const std::vector<double>& punto) {
    double suma = 0.0;
    for (size_t i = 0; i < punto.size(); ++i) {
        double coordenada = punto[i];
        suma += coordenada * coordenada;
    }
    return exp(-suma);
}

int main(int argc, char* argv[]) {

    // Verificación de parámetros ingresados por el usuario
    if(argc != 9){
        std::cerr << "Usage: " << argv[0] << " --li [límite inferior] --ls [límite superior] --d [número de dimensiones] --n [cantidad de puntos]" << std::endl;
        exit(1);
    }

    // Parámetros elegidos por el usuario
    double lim_inf, lim_sup;

    lim_inf = atof(argv[2]);
    lim_sup = atof(argv[4]);

    int dimensiones, N;

    dimensiones = atoi(argv[6]);
    N = atoi(argv[8]);

    // semilla
    unsigned int seed = 12345;

    // Variables para el cálculo
    double suma_final = 0.0;
    double suma_final2 = 0.0;  // Para calcular varianza
    double time_1 = omp_get_wtime();
    int num_procs;

    // Apertura del scope paralelo
    #pragma omp parallel
    {

    // Generador Mersenne Twister
    std::mt19937 generador(seed + 7919 * omp_get_thread_num());

    // Distribución uniforme estándar
    std::uniform_real_distribution<double> dist(lim_inf, lim_sup);

    // Inicialización de punto multidimensional
    std::vector<double> punto(dimensiones);

    // Variable para obtener número de hilos
    #pragma omp single
    num_procs = omp_get_num_threads();

    // Paralelización del for y reducción de sumas locales a sumas globales
    #pragma omp for reduction (+: suma_final, suma_final2)

    // Bucle principal de Monte Carlo
    for (int i = 0; i < N; i++) {

        for (int d = 0; d < dimensiones; d++) {
            punto[d] = dist(generador);
        }

        double valor_final = func(punto);

        suma_final += valor_final;
        suma_final2 += valor_final * valor_final;
    }

    }
    double time_2 =  omp_get_wtime();

    // Cálculos finales
    double promedio = suma_final / N;
    double promedio_cuadrado = suma_final2 / N;
    double varianza = promedio_cuadrado - promedio * promedio;

    // Volumen del hipercubo
    double volumen = 1.0;
    for (int d = 0; d < dimensiones; d++) {
        volumen *= (lim_sup - lim_inf);
    }

    double integral = promedio * volumen;
    double error = volumen * std::sqrt(varianza / N);

    std::cout << "Dimensión que utilizaremos: " << dimensiones << std::endl;
    std::cout << "Número de puntos: " << N << std::endl;
    std::cout << "Límites: (" << lim_inf << ", " << lim_sup << ")" << std::endl;
    std::cout << "RESULTADOS:" << std::endl;
    std::cout << "Integral estimada: " << integral << std::endl;
    std::cout << "Error estimado: " << error << std::endl;
    std::cout << "Varianza de f: " << varianza << std::endl;
    std::cout << "Escala del error: O(N^{-1/2})" << std::endl;
    std::cout << "Tiempo: " << time_2 - time_1 << std::endl;

    return 0;
}
