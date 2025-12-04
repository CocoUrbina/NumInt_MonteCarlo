# Explicación

Este documento presenta el fundamento matemático, computacional y conceptual del proyecto.  
Aquí se explica el método de Monte Carlo, su convergencia, la función utilizada, la estructura general del algoritmo, y se señalan los apartados donde se integrarán las extensiones del proyecto relacionadas con **memoria compartida**, **memoria distribuida** y **escalabilidad paralela**.

---

## 1. Motivación del método de Monte Carlo

La integración numérica tradicional (trapecio, Simpson, cuadraturas gaussianas) escala de forma muy pobre con la dimensión del espacio.  
Si se quiere calcular

$$
I = \int_{\Omega} f(\mathbf{x}) \, d\mathbf{x},
$$

y $\Omega$ es un dominio $d$-dimensional, los métodos deterministas requieren una cantidad de puntos proporcional a $N^d$ (fenómeno conocido como *curse of dimensionality*).

Monte Carlo evita esta explosión combinatoria mediante el uso de **muestras aleatorias**, lo que permite que el error dependa principalmente de la cantidad de puntos y **no** de la dimensión.

---

## 2. Fundamento matemático: integral como valor esperado

Consideremos un hipercubo $[a,b]^d$ y definamos su volumen como

$$
V = (b - a)^d.
$$

Si $\mathbf{X}$ es un vector aleatorio uniforme en dicho dominio, entonces la integral puede reescribirse como:

$$
I = \int_{[a,b]^d} f(\mathbf{x})\,d\mathbf{x} = V \, \mathbb{E}[f(\mathbf{X})].
$$

Por lo tanto, si tomamos $N$ muestras independientes $\mathbf{X}_1, \ldots, \mathbf{X}_N$, podemos estimar:

$$
\hat{I}_N = V \frac{1}{N} \sum_{i=1}^{N} f(\mathbf{X}_i).
$$

Esta fórmula es la base del código utilizado.

---

## 3. Convergencia del método

Una propiedad clave de Monte Carlo es que el error decrece según:

$$
\Delta I \sim O(N^{-1/2}),
$$

sin importar la dimensión $d$.  
Esto hace al método muy atractivo para integrales de alta dimensión.

En la práctica, el error se estima como:

$$
\Delta I = V \sqrt{\frac{\sigma^2}{N}},
$$

donde

$$
\sigma^2 = \mathbb{E}[f^2] - (\mathbb{E}[f])^2.
$$

En el programa, se calcula esto mediante:

- `suma_final` → aproximación de $\mathbb{E}[f]$  
- `suma_final2` → aproximación de $\mathbb{E}[f^2]$

---

## 4. Función objetivo utilizada

Para las pruebas se escogió:

$$
f(\mathbf{x}) = \exp\left(-\sum_{i=1}^d x_i^2\right),
$$

una función suave que aparece en:

- probabilidad multivariada,  
- física estadística,  
- integrales gaussianas de alta dimensión.

Es ideal para evaluar el rendimiento del método.

---

## 5. Generación de números aleatorios

El proyecto utiliza el generador **MT19937 (Mersenne Twister)** por su alta calidad estadística.

Una coordenada uniforme entre 0 y 1 se genera como:

```cpp
double r = double(generador()) / double(generador.max());
```


Luego se escala al intervalo arbitrario $[a,b]$ mediante:

$$
x_i = a + (b - a) r.
$$

Este proceso se repite para cada dimensión y para cada punto de integración.

---

## 6. Estructura general del algoritmo

El algoritmo implementado sigue los pasos clásicos del método de Monte Carlo:

1. Elegir los parámetros del problema:  
   número de puntos $N$, número de dimensiones $d$, y límites del dominio.
2. Inicializar el generador pseudoaleatorio (MT19937).
3. Para cada iteración $i = 1, \ldots, N$:
   - Generar un punto $\mathbf{X}_i$ en dimensión $d$.  
   - Evaluar la función objetivo $f(\mathbf{X}_i)$.  
   - Acumular $f(\mathbf{X}_i)$ y $f(\mathbf{X}_i)^2$.
4. Calcular:  
   - el valor medio aproximado $\hat{f}$,  
   - la varianza aproximada $\hat{\sigma}^2$,  
   - la integral estimada $\hat{I}_N$,  
   - y el error estadístico asociado.
5. Imprimir los resultados.

El estimador final toma la forma:

$$
\hat{I}_N = V \, \hat{f},
$$

donde $V$ es el volumen del dominio de integración.

---

## 7. Sobre las dos versiones del código usadas en el proyecto

El proyecto contiene dos implementaciones del mismo algoritmo:

### (1) Código estático

En esta versión, los parámetros están fijados dentro del programa:
```cpp
int N = 10000000;
int dimensiones = 3;
double lim_inf = 0.0;
double lim_sup = 1.0;
```
Este enfoque es útil para pruebas rápidas, reproducibilidad y debugging inicial.

---

### (2) Código interactivo (parsing de argumentos)

Esta versión permite al usuario especificar los valores directamente desde la línea de comandos:

./mc --li 0 --ls 1 --d 3 --n 10000000

Esto se maneja mediante:
```cpp
if (argc != 9) {
    std::cerr << "Usage: ..." << std::endl;
    exit(1);
}
```
y posterior extracción con `atof()` y `atoi()`.

La ventaja del enfoque interactivo es que permite realizar múltiples experimentos sin re-compilar el programa.

---

### ¿Cuál versión debe describirse en esta sección?

Ambas implementaciones realizan el mismo método numérico.  
En **Explanation.md** debe explicarse:

- el método matemático,  
- el flujo general del algoritmo,  
- las ideas principales.

Los detalles sobre el uso de argumentos deben documentarse en:

- `tutorials.md` (uso práctico),  
- `reference.md` (API e interfaz del programa).

---

## 8. Sección reservada: paralelismo con memoria compartida (OpenMP)

> **Esta sección se completará cuando se integre el código paralelo.**

Aquí debe incluirse:

- El concepto de dividir el ciclo Monte Carlo entre múltiples hilos.  
- Cómo funciona una cláusula `reduction(+:suma_final, suma_final2)`.  
- Problemas de condiciones de carrera y reproducibilidad.  
- Límites prácticos del speedup debido a saturación de núcleos.

Lo que NO debe incluirse aquí:

- Código completo de OpenMP.  
- Benchmarking detallado.  
- Flags de compilación.

---

## 9. Sección reservada: paralelismo con memoria distribuida (MPI)

> **Esta sección se completará cuando se implemente MPI.**

Aquí debe explicarse:

- Cómo se divide el trabajo total $N$ entre procesos independientes.  
- Cómo cada proceso computa su estimador parcial:  
  $$\hat{f}_p = \frac{1}{N_p}\sum_{i=1}^{N_p} f(\mathbf{X}_{i,p})$$  
- Cómo se combinan los resultados mediante `MPI_Reduce` o `MPI_Allreduce`.  
- Cómo influyen latencia y ancho de banda en el rendimiento.

NO incluir aquí:

- Código de MPI,  
- Tablas de resultados,  
- Scripts de ejecución.

---

## 10. Sección reservada: análisis de escalabilidad

> **Aquí se incluirá el análisis después de realizar las pruebas paralelas.**

Debe contener:

- Speedup:  
  $$S(p) = \frac{T(1)}{T(p)}$$  
- Eficiencia:  
  $$E(p) = \frac{S(p)}{p}$$  
- Discusión sobre por qué Monte Carlo escala bien.  
- Comparación entre memoria compartida vs. distribuida.  
- Impacto del costo de comunicación en MPI.

NO debe incluir:

- Tablas crudas,  
- Código repetido,  
- Gráficos directos (pueden ir en sección aparte).

---

## 11. Conclusión conceptual

El método de Monte Carlo provee:

- simplicidad en implementación,  
- independencia del error respecto a la dimensión,  
- facilidad para paralelizar,  
- control estadístico del error.

Las dos versiones del código (estática e interactiva) brindan flexibilidad tanto para pruebas internas como para experimentación en entornos paralelos.

