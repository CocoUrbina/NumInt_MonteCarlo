# Tutorials

Este documento explica cómo ejecutar, modificar y acelerar el programa de integración Monte Carlo usando sus cuatro versiones: secuencial interactiva, secuencial con argumentos, paralela con OpenMP y paralela con MPI. También incluye todos los experimentos necesarios para cumplir los milestones del proyecto.

---

# 1. Versión secuencial con parámetros internos (MonteCarlo.cpp)

## Compilación
```bash
g++ -O3 -std=c++17 MonteCarlo.cpp -o MonteCarlo
```

## Ejecución
```bash
./MonteCarlo
```

## Experimentos sugeridos (Milestone 1)
- Modificar N en el archivo: 1e5, 1e6, 1e7.
- Modificar dimensiones: 2, 3, 4, ...
- Ejecutar y registrar:
  - Integral estimada
  - Varianza
  - Error estimado
- Observar mejora al aumentar N.

---

# 2. Versión secuencial con argumentos (MonteCarlo2.cpp)

## Compilación
```bash
g++ -O3 -std=c++17 MonteCarlo2.cpp -o MonteCarlo2
```

## Ejecución
```bash
./MonteCarlo2 --li <lim_inf> --ls <lim_sup> --d <dim> --n <puntos>
```

## Ejemplos
```bash
./MonteCarlo2 --li 0 --ls 1 --d 3 --n 10000
./MonteCarlo2 --li 0 --ls 1 --d 3 --n 100000
./MonteCarlo2 --li 0 --ls 1 --d 3 --n 1000000
```

## Estudio del error (Milestone 2)
1. Fijar d = 3.
2. Probar N = 1e4, 1e5, 1e6, 1e7.
3. Registrar el error.
4. Graficar log(error) vs log(N).
5. Pendiente ≈ -1/2 confirma teoría Monte Carlo.

---

# 3. Versión con memoria compartida (ParalelizacionMC.cpp)

Cambios principales respecto al código secuencial:
- Cada hilo usa su propio generador: `std::mt19937 gen(seed + tid * 7919);`
- Se usa distribución uniforme estándar.
- Paralelización del bucle con reducción:
  ```cpp
  #pragma omp for reduction(+: suma_final, suma_final2)
  ```
- Se mide el tiempo con `omp_get_wtime()`.

## Compilación
```bash
g++ -O3 -std=c++17 -fopenmp ParalelizacionMC.cpp -o ParalelizacionMC
```

## Ejecución con distinto número de hilos
```bash
export OMP_NUM_THREADS=1
./ParalelizacionMC --li 0 --ls 1 --d 3 --n 10000000

export OMP_NUM_THREADS=2
./ParalelizacionMC --li 0 --ls 1 --d 3 --n 10000000

export OMP_NUM_THREADS=4
./ParalelizacionMC --li 0 --ls 1 --d 3 --n 10000000

export OMP_NUM_THREADS=8
./ParalelizacionMC --li 0 --ls 1 --d 3 --n 10000000
```

## Escalamiento (Milestone 3)
Calcular:
- Speedup:  
  $$ S(p) = \frac{T_1}{T_p} $$
- Eficiencia:  
  $$ E(p) = \frac{S(p)}{p} $$

Registrar:
- Tiempos
- Speedup
- Eficiencia
- Número de hilos usados

---

# 4. Versión con MPI (ParalelizacionMD.cpp)

Cambios principales:
- División del trabajo entre procesos usando nlocal.
- Semilla distinta por proceso: `seed + rank * 1234567`.
- Uso de MPI_Reduce para sumar resultados parciales:
  ```cpp
  MPI_Reduce(&s_local, &s_global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  ```
- Medición del tiempo con MPI_Wtime().

## Compilación
```bash
mpic++ -O3 -std=c++17 ParalelizacionMD.cpp -o ParalelizacionMD
```

## Ejecución
```bash
mpirun -np 1 ./ParalelizacionMD --li 0 --ls 1 --d 3 --n 10000000
mpirun -np 2 ./ParalelizacionMD --li 0 --ls 1 --d 3 --n 10000000
mpirun -np 4 ./ParalelizacionMD --li 0 --ls 1 --d 3 --n 10000000
mpirun -np 8 ./ParalelizacionMD --li 0 --ls 1 --d 3 --n 10000000
```

## Escalamiento distribuido (Milestone 4)

Registrar:
- Tiempo del proceso 0.
- Calcular speedup y eficiencia igual que OpenMP.
- Comentar diferencias entre OpenMP y MPI:
  - MPI escala mejor para N grandes.
  - Para N pequeños, MPI tiene overhead significativo.

---

# 5. Lista final de experimentos obligatorios

## (A) Error Monte Carlo
- Ejecutar MonteCarlo2 para varios N.
- Graficar log(error) vs log(N).
- Verificar pendiente ≈ −0.5.

## (B) Escalamiento en OpenMP
- Fijar N grande.
- Probar p = 1, 2, 4, 8 hilos.
- Calcular speedup y eficiencia.

## (C) Escalamiento en MPI
- Fijar N grande.
- Probar np = 1, 2, 4, 8 procesos.
- Comparar con OpenMP.

---

# 6. Resumen

Este tutorial contiene:
- Cómo ejecutar cada versión del programa.
- Qué modificar para los experimentos.
- Cómo medir tiempos.
- Cómo obtener speedup y eficiencia.
- Cómo verificar el comportamiento del error.
- Qué incluir en el informe final del proyecto.


