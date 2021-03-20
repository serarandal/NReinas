#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include  "mpi.h"

#define TRUE  1
#define FALSE 0
#define MAX_REINAS 25 
#define LONG_BUFFER 10000

int reinaEnFila[MAX_REINAS];

int numReinas, soluciones=0;

int aceptable (int reinaFila, int reinaColumna) {
int col;
  
  if (reinaEnFila[reinaColumna]!=0) return FALSE; 
  for(col=1; col<=numReinas; col++) 
    if (   ((reinaEnFila[col] != 0)
        && (abs(reinaEnFila[col]-reinaFila))==(abs(reinaColumna-col))))
      return FALSE;
  return TRUE;
}

//-------------------------------------------------------------------+
int NReinas(int reina) {
int fila,columna,col; 
 
 
  
  for(col=1;col<=numReinas;col++) {
    if(aceptable(reina,col)) {
      reinaEnFila[col]=reina; 
      if (reina==numReinas) {
        if (soluciones==0) {
          printf("\n\n");
          for (fila=1;fila<=numReinas;fila++) {
            for(columna=1;columna<=numReinas;columna++) {
              if (fila!=reinaEnFila[columna]) printf(" *");
              else printf(" Q");
            }
            printf("\n");
          }
        } 
        soluciones++;
      }
      else NReinas(reina+1);
      reinaEnFila[col]=0;
    }
  }
  return soluciones;
}
	
//--------------------------------------------------------------------
void esclavo(int yo) {
 int numsoluciones;
 reinaEnFila[yo]=1;
numsoluciones = NReinas(2);
 MPI_Send(&numsoluciones,1,MPI_INT,0,1,MPI_COMM_WORLD);
}
 
//-------------------------------------------------------------------
void maestro(int numEsclavos){
struct timeval t0, tf, t;
int numeroSoluciones = 0; 
int buffer;
MPI_Status estado;
  gettimeofday (&t0, NULL);
  for(int i=0;i<numEsclavos;i++){
  MPI_Recv(&buffer,1,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&estado);
  numeroSoluciones += buffer;
  }	
  gettimeofday (&tf, NULL);
  timersub (&tf, &t0, &t);
  printf("Numero de solucionesFinal: %d \n",numeroSoluciones);
  printf ("Tiempo => %ld:%ld seg:miliseg\n", t.tv_sec, t.tv_usec/1000);
}

//------------------------------------------------------------------
int main (int argc, char *argv[])
{
  int i, yo, numProcesos;
  
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &yo);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcesos);
  if (argc != 2) {
    if (yo == 0) printf ("Uso: nreinaspar numReinas\n");
    return 0;
  }
  numReinas = atoi(argv[1]);
  if (numReinas >= MAX_REINAS) {
    if (yo == 0) printf ("Error: El numero de reinas no puede superar %d\n", (MAX_REINAS-1));
    return 0;
  }
  for (i=0; i<MAX_REINAS; i++) reinaEnFila[i] = 0;
  soluciones=0;
  if (yo == 0) maestro(numProcesos-1);
  else         esclavo(yo);
  MPI_Finalize();
  return 0;
}
