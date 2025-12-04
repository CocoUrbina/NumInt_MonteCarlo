# Documentación

Documentación técnica del programa de integración Monte Carlo.

## Uso de la línea de comando

El programa se ejecuta como:

``` bash
./mc --li <lim_inf> --ls <lim_sup> --d <dim> --n <puntos>
```

## Argumentos

  Argumento   Descripción
  ----------- -------------------------------
  `--li`      Límite inferior del intervalo
  `--ls`      Límite superior
  `--d`       Número de dimensiones
  `--n`       Cantidad de puntos

Ejemplo:

``` bash
./mc --li -2 --ls 2 --d 4 --n 500000
```

------------------------------------------------------------------------

## Función integrada

``` cpp
double func(const std::vector<double>& punto);
```

Evalúa:

$$
f(x) = e^{-(x_1^2 + \dots + x_d^2)}
$$

------------------------------------------------------------------------

## Generación de puntos aleatorios

El código usa un generador Mersenne Twister:

``` cpp
std::mt19937 generador(seed);
double r = double(generador()) / generador.max();
```

Luego escala el número uniforme al intervalo:

``` cpp
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

En código:

``` cpp
double varianza = promedio_cuadrado - promedio * promedio;
```

------------------------------------------------------------------------

## Impresión final

El programa muestra:

-   integral estimada\
-   error\
-   varianza\
-   escala del error

