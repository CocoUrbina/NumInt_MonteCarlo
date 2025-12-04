# Introducción

Este proyecto implementa y acelera la integración numérica mediante el método de **Monte Carlo**, usando:

- **Memoria compartida** (paralelismo en un solo nodo).
- **Memoria distribuida** (paralelismo entre varios nodos o procesos).
- Integrales **multidimensionales** y estudio del **error numérico**.

En esta página se presenta la motivación general y la formulación básica del método.  
Los detalles conceptuales se amplían en la sección **Explicación**, los ejemplos paso a paso en **Ejemplos de uso** y la documentación de funciones en **Documentación**.

---

## Integración numérica con Monte Carlo

La integración con los métodos de Monte Carlo constituye un pilar de las técnicas de matemáticas aplicadas.  
La idea corresponde a estimar el **valor medio** de una función para aproximar una integral usando números aleatorios.

Considere la integral definida en el intervalo $[a,b]$:

$$
I = \int_a^b f(x)\,dx.
$$

El valor promedio o valor medio de la función $f$ se define mediante

$$
\langle f \rangle = \frac{1}{b-a} \int_a^b f(x)\,dx
$$

y, por lo tanto,

$$
I = \langle f \rangle (b-a).
$$

La idea del método de Monte Carlo es estimar \(\langle f \rangle\) usando \(N\) números pseudoaleatorios, mediante la aproximación

$$
\langle f \rangle \approx \frac{1}{N} \sum_{i=1}^{N} f(x_i)
\quad \Longrightarrow \quad
I = \frac{b-a}{N} \sum_{i=1}^{N} f(x_i),
$$

donde los $x_i$ son números pseudoaleatorios en el intervalo $a,b$.  
La técnica de Monte Carlo es particularmente poderosa para evaluar integrales en **muchas dimensiones**, ya que el costo crece de forma más suave que en los métodos deterministas tradicionales.

---

## Objetivos del proyecto (Milestones)

- Emplear la técnica de Monte Carlo de punto medio para evaluar una integral multidimensional.
- Estudiar el error como función de $N$ para comprobar la escalabilidad del método.
- Utilizar el paralelismo de **memoria compartida** para acelerar el procedimiento.
- Utilizar el paralelismo de **memoria distribuida** para acelerar el procedimiento.
- Evaluar la **escalabilidad** en cada caso y comparar los resultados.

---

## Navegación

- **Explición**: detalle teórico del método, derivaciones y consideraciones numéricas.
- **Ejemplos de uso**: ejemplos completos de uso, desde casos simples hasta integrales multidimensionales.
- **Documentación**: API y descripción formal de las funciones e interfaces del código.

