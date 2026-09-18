// Ejercicio A: Paso de arreglos en anillo
// Autores: José Luis Rodríguez, Jose Eduardo Martinez
// Materia: Sistemas Distribuidos
// Profesora: Elba Karen Sáenz García

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int id, np;
    MPI_Status estado;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    int nelements = 3; 
    int max_elements = nelements + np; 
    
    // Reserva de memoria dinámica para el arreglo original más los agregados
    int *A = (int *)malloc(max_elements * sizeof(int));
    float *B = (float *)malloc(max_elements * sizeof(float));
    int count;

    if (id == 0) {
        // P0 inicializa y muestra los valores originales
        count = nelements;
        for (int i = 0; i < count; i++) {
            A[i] = i + 1;
            B[i] = (float)(i + 1) * 1.5;
        }

        printf("\n===== [Proceso 0] Valores Iniciales =====\n");
        printf("A: "); for(int i=0; i<count; i++) printf("%d ", A[i]); printf("\n");
        printf("B: "); for(int i=0; i<count; i++) printf("%.2f ", B[i]); printf("\n");
        fflush(stdout);

        A[count] = 10;
        B[count] = 10.5;
        count++;

        MPI_Send(&count, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(A, count, MPI_INT, 1, 1, MPI_COMM_WORLD);
        MPI_Send(B, count, MPI_FLOAT, 1, 2, MPI_COMM_WORLD);

        // P0 cierra el anillo esperando la respuesta del último proceso
        MPI_Recv(&count, 1, MPI_INT, np - 1, 0, MPI_COMM_WORLD, &estado);
        MPI_Recv(A, count, MPI_INT, np - 1, 1, MPI_COMM_WORLD, &estado);
        MPI_Recv(B, count, MPI_FLOAT, np - 1, 2, MPI_COMM_WORLD, &estado);

        printf("\n===== [Proceso 0] Resultado Final (despues del anillo) =====\n");
        printf("A: "); for(int i=0; i<count; i++) printf("%d ", A[i]); printf("\n");
        printf("B: "); for(int i=0; i<count; i++) printf("%.2f ", B[i]); printf("\n\n");
        fflush(stdout);

    } else {
        MPI_Recv(&count, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &estado);
        MPI_Recv(A, count, MPI_INT, id - 1, 1, MPI_COMM_WORLD, &estado);
        MPI_Recv(B, count, MPI_FLOAT, id - 1, 2, MPI_COMM_WORLD, &estado);

        int nuevo_A = (id + 1) * 10;
        float nuevo_B = (float)((id + 1) * 10) + 0.5;
        
        A[count] = nuevo_A;
        B[count] = nuevo_B;
        count++;

        printf("\n[Proceso %d] Arreglos recibidos. Elementos agregados: A->%d, B->%.2f\n", id, nuevo_A, nuevo_B);
        printf("[Proceso %d] Arreglo A actual: ", id);
        for(int i=0; i<count; i++) printf("%d ", A[i]); printf("\n");
        printf("[Proceso %d] Arreglo B actual: ", id);
        for(int i=0; i<count; i++) printf("%.2f ", B[i]); printf("\n");
        fflush(stdout);

        // Envío al siguiente proceso (o al P0 si es el último)
        int destino = (id == np - 1) ? 0 : id + 1;
        
        MPI_Send(&count, 1, MPI_INT, destino, 0, MPI_COMM_WORLD);
        MPI_Send(A, count, MPI_INT, destino, 1, MPI_COMM_WORLD);
        MPI_Send(B, count, MPI_FLOAT, destino, 2, MPI_COMM_WORLD);
    }

    free(A);
    free(B);
    MPI_Finalize();
    return 0;
}