#!/bin/bash

echo "Compilando MPI..."
mpic++ MC_MemDist_mpi.cpp -o MC_MemDist_mpi.x || { echo "Error al compilar"; exit 1; }

MAX_PROCS=8

[ -f scaling_mpi.dat ] && rm scaling_mpi.dat

echo "Ejecutando pruebas MPI..."
for (( p=1; p<=${MAX_PROCS}; p++ )); do
    echo "Con $p procesos..."

    TIEMPO=$(mpirun -np $p ./MC_MemDist_mpi.x --li 0 --ls 1 --d 3 --n 10000000               | grep "Tiempo:" | awk '{print $2}')

    echo "$p $TIEMPO" >> scaling_mpi.dat
done

T1=$(head -n 1 scaling_mpi.dat | awk '{print $2}')

awk -v T1="$T1" '{printf "%d %.6f\n", $1, T1/$2}' scaling_mpi.dat > speedupMC_MemDist.dat

echo "Listo: speedupMC_MemDist.dat generado."

if [ -f fig.plt ]; then
    echo "Generando figura..."
    sed 's/speedupFourier.dat/speedupMC_MemDist.dat/' fig.plt | gnuplot
fi

echo "Finalizado."
