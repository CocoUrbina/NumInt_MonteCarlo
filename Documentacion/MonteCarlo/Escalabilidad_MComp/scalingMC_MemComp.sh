#!/bin/bash

echo "Compilando maxThreads.cpp..."
g++ maxThreads.cpp -o maxThreads.x -fopenmp || { echo "Error"; exit 1; }

MAX_THREADS=$(./maxThreads.x | awk '{print $3}')
echo "Máximo número de hilos: $MAX_THREADS"

echo "Compilando MC_MemComp.cpp..."
g++ MC_MemComp.cpp -o MC_MemComp.x -fopenmp || { echo "Error"; exit 1; }

[ -f scaling.dat ] && rm scaling.dat

echo "Ejecutando pruebas..."
for (( t=1; t<=${MAX_THREADS}; t++ )); do
    echo "Con $t hilos..."

    TIEMPO=$(OMP_NUM_THREADS=$t ./MC_MemComp.x --li 0 --ls 1 --d 3 --n 10000000              | grep "Tiempo:" | awk '{print $2}')

    echo "$t $TIEMPO" >> scaling.dat
done

T1=$(head -n 1 scaling.dat | awk '{print $2}')

awk -v T1="$T1" '{printf "%d %.6f\n", $1, T1/$2}' scaling.dat > speedupMC_MemComp.dat

if [ -f fig.plt ]; then
    echo "Generando figura..."
    gnuplot fig.plt
fi

echo "Finalizado."
