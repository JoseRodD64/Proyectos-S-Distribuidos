// Ejercicio 2: Distribucion de Matriz y Suma de Renglones 
// Autores: José Luis Rodríguez, Jose Eduardo Martinez
// Materia: Sistemas Distribuidos
// Profesora: Elba Karen Sáenz García

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 8
#define M 4

int main(int argc, char** argv) {
    int id, np;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (N % np != 0) {
        if (id == 0) printf("Error: N (%d) debe ser divisible entre np (%d).\n", N, np);
        MPI_Finalize();
        return 1;
    }

    int filas_locales = N / np;
    int elementos_locales = filas_locales * M;

    int *matriz = NULL;
    int *bloque = (int *)malloc(elementos_locales * sizeof(int));

    if (id == 0) {
        matriz = (int *)malloc(N * M * sizeof(int));
        printf("\n===== Matriz Inicial A =====\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                matriz[i * M + j] = (i + 1) * 2 + j; 
                printf("%4d ", matriz[i * M + j]);
            }
            printf("\n");
        }
        printf("============================\n");
        fflush(stdout);
    }

    // El proceso 0 distribuye la matriz
    MPI_Scatter(matriz, elementos_locales, MPI_INT, bloque, elementos_locales, MPI_INT, 0, MPI_COMM_WORLD);

    printf("\n[Proceso %d] Analizando datos recibidos:\n", id);
    
    int min_suma_local = -1;
    int fila_min_local = -1;

    // Desglose de los calculos por cada proceso
    for (int i = 0; i < filas_locales; i++) {
        int suma_actual = 0;
        printf("  -> [Proceso %d] Valores locales (", id);
        
        for (int j = 0; j < M; j++) {
            suma_actual += bloque[i * M + j];
            printf("%d ", bloque[i * M + j]);
        }

        int fila_global = (id * filas_locales) + i;
        printf(") | Suma = %d (Renglon global %d)\n", suma_actual, fila_global);

        if (i == 0 || suma_actual < min_suma_local) {
            min_suma_local = suma_actual;
            fila_min_local = fila_global;
        }
    }
    
    printf("[Proceso %d] --- Reporta menor suma local: %d (Renglon %d) ---\n", id, min_suma_local, fila_min_local);
    fflush(stdout);

    int *mins_globales = NULL;
    int *filas_mins = NULL;

    if (id == 0) {
        mins_globales = (int *)malloc(np * sizeof(int));
        filas_mins = (int *)malloc(np * sizeof(int));
    }

    // Se recolectan los minimos locales
    MPI_Gather(&min_suma_local, 1, MPI_INT, mins_globales, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&fila_min_local, 1, MPI_INT, filas_mins, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (id == 0) {
        int min_absoluto = mins_globales[0];
        int f_min_abs = filas_mins[0];

        // P0 muestra que recibio de cada proceso antes del veredicto
        printf("\n===== Reporte Consolidado en P0 =====\n");
        for (int p = 0; p < np; p++) {
            printf("Recibido de P%d -> Renglon %d con suma %d\n", p, filas_mins[p], mins_globales[p]);
            
            if (mins_globales[p] < min_absoluto) {
                min_absoluto = mins_globales[p];
                f_min_abs = filas_mins[p];
            }
        }

        printf("-------------------------------------\n");
        printf("VEREDICTO: Renglon con MENOR suma global: %d\n", f_min_abs);
        printf("VEREDICTO: Suma de ese renglon: %d\n\n", min_absoluto);
        fflush(stdout);

        free(matriz);
        free(mins_globales);
        free(filas_mins);
    }

    free(bloque);
    MPI_Finalize();
    return 0;
}