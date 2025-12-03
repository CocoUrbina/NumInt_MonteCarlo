#include <iostream>
#include <cstdlib> //c standard library para el srand (veremos después)
#include <cmath>


//Primero la función a integrar (multidimensional como dice en milestone)
double func(double x, double y, double z){
  return exp(-(x * x + y * y + z * z));
}


//función main
int main(){
  
  //datos necesarios
  int N = 10000000;
  double lim_inf = 0.0;
  double lim_sup = 1.0;
  int dimensiones = 3;

  //ahora aquí vamos a poner el seed para que los números pseudoaleatorios se repitan en todo lugar

  std::srand(2345);

  //ahora ya con el setup, hacemos el montecarlo de puntos medios


  // creamos variable sum para ir sumando las funciones
  double sum = 0.0;
  

  //ahora for loop creo que probablmente aquí tenga que hacer la paralelización:
  for (int i = 0; i < N; i++){
  
    //ahora para números aleatorios, serán de forma uniforme (misma prob que salgan) (son números aleatorios dentro del rango de lim_inf y lim_sup

    double x = lim_inf + (lim_sup - lim_inf) * (double(std::rand()) / RAND_MAX);
    double y = lim_inf + (lim_sup - lim_inf) * (double(std::rand()) / RAND_MAX);
    double z = lim_inf + (lim_sup - lim_inf) * (double(std::rand()) / RAND_MAX);
    
//antes de lo siguiente es importante explicar que RAND_MAX es el numero aleatorio maximo, lo estoy dividiendo entre rand_max para que de TODOS los números que pueden salir según la computadora, si lo divido entre RAND_MAX quedará entre 0 y 1 de forma uniforme que es nuestro rango

    //ahora agrego a nuestro sum
    sum += func(x,y,z);
  }

//ahora estimemos el valor medio <f>
  double promedio = sum / N;

//sacamos volumen de cubo porque es integral de tres dimensiones:
  double volumen = (lim_sup - lim_inf) * (lim_sup - lim_inf) * (lim_sup - lim_inf);

//ahora finalmente saco la integral
  double integral = promedio * volumen;

  std::cout << "La integral con Monte Carlo sería:" << integral << std::endl;

  return 0;
}
