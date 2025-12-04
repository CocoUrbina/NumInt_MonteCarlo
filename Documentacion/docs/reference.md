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

\[ f(x) = e^{-(x_1^2 + `\dots `{=tex}+ x_d\^2)} \]

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

\[ `\bar`{=tex}{f} = `\frac{1}{N}`{=tex} `\sum `{=tex}f(x_i) \]

Volumen del espacio:

\[ V = (`\text{ls}`{=tex}-`\text{li}`{=tex})\^d \]

Integral estimada:

\[ I `\approx `{=tex}`\bar`{=tex}{f} `\cdot `{=tex}V \]

Error estadístico:

\[ `\text{error}`{=tex} = V`\sqrt{\frac{\mathrm{Var}(f)}{N}}`{=tex} \]

------------------------------------------------------------------------

## Cálculo de varianza

\[ `\mathrm{Var}`{=tex}(f) = `\langle `{=tex}f\^2`\rangle `{=tex}-
`\langle `{=tex}f`\rangle`{=tex}\^2 \]

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

Todo esto en un formato claro para el usuario.
