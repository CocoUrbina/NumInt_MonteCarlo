# Explicación del proyecto: Integración Monte Carlo secuencial y paralela (OpenMP + MPI)

Este documento explica el fundamento matemático, computacional y paralelo del proyecto.  
Se describen:

- El método de Monte Carlo y su convergencia.  
- La función que se integra numéricamente.  
- La estructura general del algoritmo.  
- El uso del generador Mersenne Twister (MT19937).  
- La implementación secuencial (parámetros fijos y por línea de comando).  
- La paralelización con **OpenMP (memoria compartida)**.  
- La paralelización con **MPI (memoria distribuida)**.  
- El análisis de **escalabilidad** en ambos esquemas paralelos.

---

## 1. Motivación: por qué usar Monte Carlo

La integración numérica determinista (regla del trapecio, Simpson, cuadraturas gaussianas, etc.) escala muy mal con la dimensión $d$.  
	Si se toma una malla con $N$ puntos por dimensión, el número total de puntos es $N^d$, lo que vuelve el problema intratable cuando $d$ crece (*curse of dimensionality*).

El método de Monte Carlo evita esta explosión combinatoria porque:

- El error estadístico depende de \(N\) (número de muestras) pero **no** de la dimensión.  
- La convergencia típica es

  $$
  \Delta I \sim O\left(N^{-1/2}\right),
  $$

  independientemente de $d$.

Por esto Monte Carlo es una herramienta estándar para integrales de alta dimensión.

---

## 2. Fundamento matemático del método

Consideremos la integral

$$
I = \int_{\Omega} f(\mathbf{x})\, d\mathbf{x},
$$

donde $\Omega = [a,b]^d$ es un hipercubo de dimensión $d$.  
El volumen de $\Omega$ es

$$
V = (b-a)^d.
$$

Si $\mathbf{X}$ es un vector aleatorio uniforme en $\Omega$, se cumple:

$$
I = \int_{\Omega} f(\mathbf{x})\, d\mathbf{x} = V \, \mathbb{E}[f(\mathbf{X})].
$$

La idea de Monte Carlo es aproximar $\mathbb{E}[f(\mathbf{X})]$ mediante el promedio muestral.  
Tomamos $N$ vectores independientes $\mathbf{X}_1, \ldots, \mathbf{X}_N$, y definimos el estimador:

$$
\hat I_N = V\frac{1}{N}\sum_{i=1}^N f(\mathbf{X}_i).
$$

El error estadístico puede estimarse usando la varianza muestral de \(f\):

$$
\sigma^2 = \mathbb{E}[f^2] - (\mathbb{E}[f])^2.
$$

El error típico (desviación estándar del estimador) es:

$$
\Delta I \approx V \sqrt{\frac{\sigma^2}{N}}.
$$

En el programa:

- `suma_final` aproxima $\sum_{i=1}^N f(\mathbf{X}_i)$,
- `suma_final2` aproxima $\sum_{i=1}^N f(\mathbf{X}_i)^2$.

A partir de estos acumuladores se calcula:

- El promedio $\hat f = \text{suma\_final}/N$.
- El promedio de cuadrados $\widehat{f^2} = \text{suma\_final2}/N$.
- La varianza estimada $\hat\sigma^2 = \widehat{f^2} - \hat f^{\,2}$.
- El error estimado $\Delta I = V \sqrt{\hat\sigma^2/N}$.

---

## 3. Función objetivo utilizada

En las cuatro versiones del código se integra la misma función:

$$
f(\mathbf{x}) = \exp\left(-\sum_{i=1}^d x_i^2\right),
$$

donde $\mathbf{x} = (x_1, \dots, x_d)$ y $d$ es el número de dimensiones.

Esta elección tiene varias ventajas:

- Es una función suave y bien comportada en todo \(\mathbb{R}^d\).  
- Está relacionada con distribuciones gaussianas multivariadas.  
- Es representativa de problemas reales en física estadística e integración de funciones tipo gaussiana.

En el código, esta función se implementa como:
```cpp
    double func(const std::vector<double>& punto) {
        double suma = 0.0;
        for (size_t i = 0; i < punto.size(); ++i) {
            double coordenada = punto[i];
            suma += coordenada * coordenada;
        }
        return exp(-suma);
    }
```
---

## 4. Generación de números aleatorios y escalamiento al intervalo

Se utiliza el generador pseudoaleatorio **Mersenne Twister (std::mt19937)**.  
La semilla se fija explícitamente para garantizar reproducibilidad.

### 4.1 Versión secuencial y MPI (forma simple)

En algunas versiones se genera un número uniforme $[0,1]$ como:
```cpp
    double r = double(generador()) / double(generador.max());
```
Dado que para `std::mt19937` se cumple `min() = 0`, esta expresión produce valores uniformes en el intervalo $[0,1]$.

Luego se escala al intervalo $[a,b]$ mediante:

$$
x = a + (b-a)\,r.
$$

en código:
```cpp
    punto[d] = lim_inf + (lim_sup - lim_inf) * r;
```
### 4.2 Versión OpenMP (distribución explícita)

En la versión paralela con OpenMP se utiliza:

- Un generador `std::mt19937` independiente por hilo.  
- Una distribución explícita `std::uniform_real_distribution<double>(lim_inf, lim_sup)`.

Ejemplo de uso:
```cpp
    std::mt19937 generador(seed + tid * 7919);
    std::uniform_real_distribution<double> dist(lim_inf, lim_sup);

    for (int d = 0; d < dimensiones; d++) {
        punto[d] = dist(generador);
    }
```
Este esquema es estadísticamente correcto e independiente en cada hilo, evitando correlaciones entre secuencias.

---

## 5. Estructura general del algoritmo

La lógica básica es la misma en las cuatro versiones:

1. **Lectura o definición de parámetros**  
   - Límite inferior `lim_inf`.  
   - Límite superior `lim_sup`.  
   - Número de dimensiones `dimensiones`.  
   - Número de puntos `N`.

2. **Inicialización de acumuladores**  
   - `suma_final = 0.0`.  
   - `suma_final2 = 0.0`.

3. **Bucle principal de Monte Carlo**  
   Para cada punto:
   - Generar un vector `punto` de dimensión `dimensiones`.  
   - Evaluar `valor_final = func(punto)`.  
   - Acumular:
     
     - `suma_final += valor_final;`  
     - `suma_final2 += valor_final * valor_final;`.

4. **Cálculo de promedios y varianza**  

   - `promedio = suma_final / N;`  
   - `promedio_cuadrado = suma_final2 / N;`  
   - `varianza = promedio_cuadrado - promedio * promedio;`.

5. **Cálculo del volumen del dominio**  

   - `volumen = (lim_sup - lim_inf)^dimensiones`.

6. **Estimación final de la integral y del error**

   - `integral = promedio * volumen;`  
   - `error = volumen * sqrt(varianza / N);`.

7. **Impresión de resultados**

   Se muestran:
   - Dimensión.  
   - Número de puntos.  
   - Intervalo de integración.  
   - Integral estimada.  
   - Error estimado.  
   - Varianza de la función.  
   - La escala del error \(O(N^{-1/2})\).  
   - En las versiones paralelas, el tiempo de ejecución.

---

## 6. Versiones secuenciales

En el proyecto hay dos versiones secuenciales del código base.

### 6.1 Versión con parámetros fijos en el código

En esta versión:
```cpp
    int N = 10000000;
    int dimensiones = 3;
    double lim_inf = 0.0;
    double lim_sup = 1.0;
```
- Todos los parámetros están definidos dentro del `main()`.  
- Es útil para pruebas controladas, comparaciones y debugging.  
- Permite verificar la corrección del método sin preocuparse por el parsing de argumentos.

El resto del algoritmo (bucle Monte Carlo, cálculo de varianza, integral y error) sigue la estructura descrita en la sección 5.

### 6.2 Versión con parámetros desde la línea de comando

La otra versión secuencial permite al usuario elegir los parámetros al momento de ejecutar el programa:
```bash
    ./mc --li <lim_inf> --ls <lim_sup> --d <dim> --n <puntos>
```
Se valida primero que se hayan recibido exactamente 9 argumentos (`argc != 9`).  
Luego se extraen así:
```cpp
    lim_inf    = atof(argv[2]);
    lim_sup    = atof(argv[4]);
    dimensiones = atoi(argv[6]);
    N          = atoi(argv[8]);
```
Esta versión es más flexible, pues el mismo ejecutable sirve para múltiples configuraciones sin recompilar.

---

## 7. Paralelismo con memoria compartida: OpenMP

La versión con **OpenMP** explota el paralelismo en memoria compartida, es decir, varias hebras (threads) dentro de un mismo proceso y nodo.

### 7.1 Idea básica

El bucle principal de Monte Carlo es:

- Altamente paralelo: cada punto se puede simular de forma independiente.  
- Ideal para dividir el trabajo entre hilos.  

Si hay $p$ hilos, cada uno procesa aproximadamente $N/p$ puntos.

### 7.2 Estructura del código con OpenMP

La sección relevante del código es:
```cpp
    double suma_final = 0.0;
    double suma_final2 = 0.0;
    double time_1 = omp_get_wtime();
    int num_procs;

    #pragma omp parallel
    {
        int num_procs = omp_get_thread_num();

        // Semilla distinta por hilo
        std::mt19937 generador(seed + tid * 7919);

        // Distribución uniforme en [lim_inf, lim_sup]
        std::uniform_real_distribution<double> dist(lim_inf, lim_sup);

        std::vector<double> punto(dimensiones);

        // Un solo hilo obtiene el número total de hilos
        #pragma omp single
        num_procs = omp_get_num_threads();

        // Bucle Monte Carlo paralelo con reducción
        #pragma omp for reduction (+: suma_final, suma_final2)
        for (int i = 0; i < N; i++) {
            for (int d = 0; d < dimensiones; d++) {
                punto[d] = dist(generador);
            }
            double val = func(punto);
            suma_final  += val;
            suma_final2 += val * val;
        }
    }

    double time_2 =  omp_get_wtime();
```
Puntos clave:

- `#pragma omp parallel` crea un grupo de hilos que ejecutan el mismo bloque.  
- Cada hilo obtiene su identificador con `omp_get_thread_num()`.  
- La semilla se ajusta por hilo (`seed + num_procs * 7919`) para generar secuencias independientes.  
- La directiva `#pragma omp single` asegura que solo un hilo ejecute `num_procs = omp_get_num_threads();`.  
- `#pragma omp for reduction(+: suma_final, suma_final2)` reparte las iteraciones del bucle `for (int i = 0; i < N; i++)` entre los hilos, acumulando de manera segura las sumas en las variables globales mediante una reducción.

Con esto se evita cualquier condición de carrera en `suma_final` y `suma_final2`.

### 7.3 Cálculo de tiempos y resultados

El tiempo de ejecución se mide con `omp_get_wtime()` antes y después del bloque paralelo:

- `time_1 = omp_get_wtime();`  
- `time_2 = omp_get_wtime();`  

El tiempo total es `time_2 - time_1`, que se imprime junto con la integral, error y varianza.

### 7.4 Comentario de corrección

La versión OpenMP es:

- Estadísticamente correcta (cada hilo tiene RNG propio).  
- Libre de condiciones de carrera (uso de `reduction`).  
- Reproducible dado un esquema fijo de hilos y semillas.

---

## 8. Paralelismo con memoria distribuida: MPI

La versión con **MPI** reparte el trabajo entre procesos (potencialmente en diferentes nodos de un clúster).  
Cada proceso tiene su propia memoria, su propio generador y calcula una parte del total.

### 8.1 Inicialización y parámetros globales

Al inicio del `main` se encuentra:
```cpp
    MPI_Init(NULL, NULL);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
```
- `size`: número total de procesos.  
- `rank`: identificador de cada proceso (0, 1, ..., size-1).

También se registra el tiempo inicial:
```cpp
    double time_1 = MPI_Wtime();
```
### 8.2 División del trabajo entre procesos

Se reparte `N` de la siguiente manera:
```cpp
    int nlocal = N / size;
    int rest = N % size;

    if (rest && rank < rest) {
        nlocal++;
    }
```
Así, los primeros `rest` procesos reciben un punto extra para balancear mejor la carga cuando `N` no es múltiplo de `size`.

Luego se calcula el índice inicial `start` y el final `end` de cada proceso:
```cpp
    int start = nlocal * rank;

    if (rest && rank >= rest) {
        start += rest;
    }

    int end = start + nlocal;
```
Cada proceso ejecuta el bucle Monte Carlo en el rango `[start, end)`.

### 8.3 Generación aleatoria y acumulación local

Cada proceso crea su propio generador:
```cpp
    std::mt19937 generador(seed + rank * 1234567);
```
Se mantiene el mismo esquema de función `func(punto)` y se acumulan sumas locales:
```cpp
    double suma_local = 0.0;
    double suma_cuadrados_local = 0.0;
```
En el bucle:
```cpp
    for (int i = start; i < end; i++) {
        std::vector<double> punto(dimensiones);
        for (int d = 0; d < dimensiones; d++) {
            double r = double(generador()) / double(generador.max());
            punto[d] = lim_inf + (lim_sup - lim_inf) * r;
        }
        double valor_final = func(punto);
        suma_local          += valor_final;
        suma_cuadrados_local += valor_final * valor_final;
    }
```
### 8.4 Reducción global y cálculo final en el proceso 0

Las sumas locales se combinan en el proceso 0 mediante:
```cpp
    double suma_global, suma_cuadrados_global;

    MPI_Reduce(&suma_local, &suma_global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&suma_cuadrados_local, &suma_cuadrados_global, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
```
Solo el proceso de `rank == 0` calcula:

- `promedio`,  
- `varianza`,  
- `integral`,  
- `error`,  
- y mide el tiempo total:
```cpp
    double time_2 = MPI_Wtime();

    if (rank == 0) {
        // Cálculos y printf de resultados
    }
```
Finalmente, todos los procesos llaman a:
```cpp
    MPI_Finalize();
```
para cerrar el entorno MPI.

---

## 9. Escalabilidad y rendimiento

La integración por Monte Carlo es un caso típico de **cómputo "embarrassingly parallel"**:

- Cada muestra es independiente.  
- No hay comunicación necesaria durante el cálculo, solo al final (reducción).  

Esto hace que la escalabilidad sea muy buena tanto en memoria compartida como en memoria distribuida.

### 9.1 Definiciones

Sea:

- $T(1)$: tiempo de ejecución con un solo hilo/proceso.  
- $T(p)$: tiempo de ejecución con $p$ hilos/procesos.

El **speedup** se define como:

$$
S(p) = \frac{T(1)}{T(p)}.
$$

La **eficiencia** es:

$$
E(p) = \frac{S(p)}{p}.
$$

El objetivo ideal es $S(p) \approx p$ y $E(p) \approx 1$.

### 9.2 Escalabilidad con OpenMP (memoria compartida)

En la versión OpenMP:

- El cálculo se reparte entre hilos dentro de un mismo nodo.  
- No hay comunicación entre hilos durante el cálculo, sólo la reducción interna que maneja OpenMP.  
- La sobrecarga es muy baja.

Por tanto:

- Para valores grandes de $N$, es esperable un **speedup casi lineal** mientras no se saturen los núcleos físicos del procesador.  
- A partir de cierto número de hilos, la ganancia marginal disminuye por overhead de sincronización y límites de hardware.

### 9.3 Escalabilidad con MPI (memoria distribuida)

En la versión MPI:

- Cada proceso realiza una fracción $N_p$ del total de muestras.  
- Sólo se comunican al final para sumar resultados (`MPI_Reduce`).  

El costo de comunicación es muy pequeño comparado con el costo de cómputo cuando $N$ es grande, por lo que:

- El speedup también puede ser casi lineal con el número de procesos.  
- Esta versión permite escalar a múltiples nodos de un clúster, superando la limitación de memoria y núcleos de un solo nodo.

---

## 10. Conclusiones

1. El método de Monte Carlo ofrece una forma robusta de aproximar integrales de alta dimensión, con error que decae como $O(N^{-1/2})$ independientemente de la dimensión.  
2. La función implementada, $f(\mathbf{x}) = \exp(-\sum x_i^2)$, es un caso de prueba clásico y permite verificar tanto la corrección como el rendimiento del método.  
3. Las versiones secuenciales (con parámetros fijos y con argumentos de línea de comando) comparten la misma lógica numérica y sirven como base de referencia.  
4. La paralelización con OpenMP explota la memoria compartida y logra reducciones significativas en el tiempo de ejecución utilizando múltiples hilos dentro de un mismo nodo.  
5. La paralelización con MPI divide el trabajo entre procesos distribuidos, permitiendo escalar el mismo método a múltiples nodos y aprovechar clústeres de cómputo.  
6. Dado que el problema es “embarrassingly parallel”, ambas aproximaciones (OpenMP y MPI) presentan muy buena escalabilidad y se integran naturalmente en el flujo del algoritmo Monte Carlo, sin cambiar la fórmula del estimador, únicamente reduciendo el tiempo total de cómputo.

Este documento resume así la parte matemática, computacional y de paralelización del proyecto de integración Monte Carlo, coherente con las cuatro implementaciones de código (secuencial fija, secuencial interactiva, OpenMP y MPI).

