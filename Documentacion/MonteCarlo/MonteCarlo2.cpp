#include <iostream>
#include <vector>
#include <random>
#include <cmath>

// Función a integrar: e^(-(x1^2 + x2^2 + ...))
double func(const std::vector<double>& punto) {
    double suma = 0.0;
    for (size_t i = 0; i < punto.size(); ++i) {
        double coordenada = punto[i];
        suma += coordenada * coordenada;
    }
    return exp(-suma);
}

int main() {
    // aquí elijo los parámetros de todo y todo el código se ajusta a estos
    int N = 10000000;          // Número de puntos
    int dimensiones = 3;       // Número de dimensiones
    double lim_inf = 0.0;      // Límite inferior
    double lim_sup = 1.0;      // Límite superior
    
    // semilla
    unsigned int seed = 12345;
    
    // Generador Mersenne Twister 
    std::mt19937 generador(seed);
    
    // Variables para el cálculo
    double suma_final = 0.0;
    double suma_final2 = 0.0;  // Para calcular varianza
    
    // Bucle principal de Monte Carlo
    for (int i = 0; i < N; i++) {
        // Generar punto multidimensional (creo que es mejor evitar traer uniform... del std library?
        std::vector<double> punto(dimensiones);
        for (int d = 0; d < dimensiones; d++) {
            // Generar número uniforme entre 0 y 1 manualmente
            double r = double(generador() - generador.min()) / 
                       double(generador.max() - generador.min());
            
            // Escalar al rango siguiente
            punto[d] = lim_inf + (lim_sup - lim_inf) * r;
        }
        
        // Evaluar función
        double valor_final = func(punto);
        
        // Acumular para cálculos finales después
        suma_final += valor_final;
        suma_final2 += valor_final * valor_final;
    }
    
    // Cálculos finales
    double promedio = suma_final / N;
    double promedio_cuadrado = suma_final2 / N;
    double varianza = promedio_cuadrado - promedio * promedio;
    
    // Volumen del hipercubo
    double volumen = 1.0;
    for (int d = 0; d < dimensiones; d++) {
        volumen *= (lim_sup - lim_inf);
    }

    // Estimación de la integral
    double integral = promedio * volumen;

    // Estimación del error
    double error = volumen * std::sqrt(varianza / N);

    // Resultados
    std::cout << "Dimensión que utilizaremos: " << dimensiones << std::endl;
    std::cout << "Número de puntos: " << N << std::endl;
    std::cout << "Límites: (" << lim_inf << ", " << lim_sup << ")" << std::endl;
    std::cout << "RESULTADOS:" << std::endl;
    std::cout << "Integral estimada: " << integral << std::endl;
    std::cout << "Error estimado: " << error << std::endl;
    std::cout << "Varianza de f: " << varianza << std::endl;
    std::cout << "Escala del error: O(N^{-1/2})" << std::endl;
    
    return 0;
}
