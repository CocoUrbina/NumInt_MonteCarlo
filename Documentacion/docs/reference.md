# Documentación

Documentación técnica del programa de integración Monte Carlo.

---

## Uso de la línea de comando

El programa se ejecuta como:

```bash
./mc --li <lim_inf> --ls <lim_sup> --d <dim> --n <puntos>
```

## Argumentos

| Argumento | Descripción                     |
|-----------|---------------------------------|
| `--li`    | Límite inferior del intervalo   |
| `--ls`    | Límite superior                 |
| `--d`     | Número de dimensiones           |
| `--n`     | Cantidad de puntos a generar    |

Ejemplo:

```bash
./mc --li -2 --ls 2 --d 4 --n 500000
```

------------------------------------------------------------------------

## Función integrada

```cpp
double func(const std::vector<double>& punto);
```

Evalúa:

$$
f(x) = e^{-(x_1^2 + \dots + x_d^2)}
$$

------------------------------------------------------------------------

## Generación de puntos aleatorios

El programa utiliza el generador Mersenne Twister:

```cpp
std::mt19937 generador(seed);
double r = double(generador()) / generador.max();
```

Escalamiento al intervalo:

```cpp
punto[d] = lim_inf + (lim_sup - lim_inf) * r;
```

------------------------------------------------------------------------

## Estimación Monte Carlo

Promedio:

$$
\bar{f} = \frac{1}{N}\sum_{i=1}^{N} f(x_i)
$$

Volumen del espacio:

$$
V = (\text{ls} - \text{li})^d
$$

Integral estimada:

$$
I \approx \bar{f} \cdot V
$$

Error estadístico:

$$
\text{error} = V \sqrt{\frac{\mathrm{Var}(f)}{N}}
$$

------------------------------------------------------------------------

## Cálculo de varianza

$$
\mathrm{Var}(f) = \langle f^2 \rangle - \langle f \rangle^2
$$

Implementación:

```cpp
double varianza = promedio_cuadrado - promedio * promedio;
```

------------------------------------------------------------------------
------------------------------------------------------------------------

# Paralelización y Escalabilidad

A continuación se documentan las extensiones del programa que permiten acelerar la integración Monte Carlo mediante memoria compartida (OpenMP) y memoria distribuida (MPI).

---

## Paralelización con memoria compartida (OpenMP)

La versión paralela divide el bucle de Monte Carlo entre múltiples hilos que comparten memoria.

### Generador independiente por hilo
```cpp
std::mt19937 gen(seed + tid * 7919);
```
Cada hilo usa una semilla distinta para evitar secuencias correlacionadas.

### Distribución uniforme estándar
```cpp
std::uniform_real_distribution<double> dist(lim_inf, lim_sup);
```

### Paralelización del bucle principal
```cpp
#pragma omp for reduction(+: suma_final, suma_final2)
for (int i = 0; i < N; i++) {
    ...
}
```

### Medición de tiempo paralelo
```cpp
double t1 = omp_get_wtime();
double t2 = omp_get_wtime();
```

### Observaciones
- El paralelismo es de tipo **data-parallel**: cada hilo procesa puntos diferentes.  
- `reduction` garantiza acumulación correcta.  
- OpenMP es ideal para máquinas de múltiples núcleos en un solo nodo.

---

## Paralelización con memoria distribuida (MPI)

La versión MPI reparte la carga de trabajo entre procesos independientes, cada uno con su propio espacio de memoria.

### División del trabajo
```cpp
int nlocal = N / size;
if (rank < rest) nlocal++;
```

### Semilla distinta por proceso
```cpp
std::mt19937 gen(seed + rank*1234567);
```

### Reducción de resultados
```cpp
MPI_Reduce(&suma_local, &suma_global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
MPI_Reduce(&suma_cuadrados_local, &suma_cuadrados_global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
```

### Medición del tiempo distribuido
```cpp
double t1 = MPI_Wtime();
double t2 = MPI_Wtime();
```

### Observaciones
- MPI escala mejor en clusters o sistemas multinodo.  
- El proceso 0 combina los resultados y produce la salida final.  
- Para tamaños pequeños de N, el overhead de comunicación puede dominar.

---

## Métricas de rendimiento y escalabilidad

Para evaluar el desempeño de OpenMP y MPI se usan las métricas estándar:

### Speedup
$$
S(p) = \frac{T_1}{T_p}
$$

### Eficiencia
$$
E(p) = \frac{S(p)}{p}
$$

### Escalamiento fuerte

El tamaño del problema $N$ es constante y se aumenta el número de hilos/procesos.

### Escalamiento débil

Se incrementa $N$ proporcionalmente al número de hilos/procesos, manteniendo el trabajo por unidad constante.

---

## Consideraciones adicionales

### Reproducibilidad
- OpenMP:  
  ```cpp
  seed + tid * 7919
  ```
- MPI:  
  ```cpp
  seed + rank * 1234567
  ```

### Estabilidad numérica
- Usar `double` para acumuladores.  
- Reducir valores en paralelo usando `reduction` o `MPI_Reduce`.

### Observaciones prácticas finales
- OpenMP requiere menos cambios y es más eficiente para una sola máquina.  
- MPI es más escalable para computación distribuida.  
- Monte Carlo escala de forma casi ideal porque cada punto es independiente.

------------------------------------------------------------------------

