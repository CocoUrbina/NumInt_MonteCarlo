#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <mpi.h>

double func(const std::vector<double>& punto) {
    double suma = 0.0;
    for (size_t i = 0; i < punto.size(); ++i) {
        double coordenada = punto[i];
        suma += coordenada * coordenada;
    }
    return exp(-suma);
}

int main(int argc, char* argv[]) {

    if(argc != 9){
        std::cerr << "Usage: " << argv[0] 
                  << " --li [límite inferior] --ls [límite superior] "
                  << "--d [número de dimensiones] --n [interacciones]" 
                  << std::endl;
        exit(1);
    }

    double lim_inf = atof(argv[2]);
    double lim_sup = atof(argv[4]);
    int dimensiones = atoi(argv[6]);
    int N = atoi(argv[8]);

    unsigned int seed = 12345;

    int size, rank;
    MPI_Init(NULL, NULL);

    double time_1 = MPI_Wtime();

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int nlocal = N / size;
    int rest = N % size;

    if (rest && rank < rest) nlocal++;

    int start = nlocal * rank;
    if (rest && rank >= rest) start += rest;

    int end = start + nlocal;

    std::mt19937 generador(seed + rank * 1234567);

    double suma_local = 0.0;
    double suma_cuadrados_local = 0.0;

    for (int i = start; i < end; i++) {
        std::vector<double> punto(dimensiones);
        for (int d = 0; d < dimensiones; d++) {
            double r = double(generador()) / double(generador.max());
            punto[d] = lim_inf + (lim_sup - lim_inf) * r;
        }

        double valor_final = func(punto);
        suma_local += valor_final;
        suma_cuadrados_local += valor_final * valor_final;
    }

    double suma_global, suma_cuadrados_global;

    MPI_Reduce(&suma_local, &suma_global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&suma_cuadrados_local, &suma_cuadrados_global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double time_2 = MPI_Wtime();

    if (rank == 0) {

        double promedio = suma_global / N;
        double promedio_cuadrado = suma_cuadrados_global / N;
        double varianza = promedio_cuadrado - promedio * promedio;

        double volumen = 1.0;
        for (int d = 0; d < dimensiones; d++)
            volumen *= (lim_sup - lim_inf);

        double integral = promedio * volumen;
        double error = volumen * std::sqrt(varianza / N);

        std::cout << "Dimensión: " << dimensiones << std::endl;
        std::cout << "Puntos: " << N << std::endl;
        std::cout << "Límites: (" << lim_inf << ", " << lim_sup << ")" << std::endl;
        std::cout << "RESULTADOS:" << std::endl;
        std::cout << "Integral estimada: " << integral << std::endl;
        std::cout << "Error estimado: " << error << std::endl;
        std::cout << "Varianza: " << varianza << std::endl;
        std::cout << "Tiempo: " << time_2 - time_1 << std::endl;
    }

    MPI_Finalize();

    return 0;
}
