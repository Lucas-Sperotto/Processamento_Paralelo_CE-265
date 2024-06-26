#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

	void InitTabuleiroLocal(int lin, int col, int **tabulIn, int **tabulOut);
	
	void InitTabuleiroGlobal(int tam, int **tabulIn, int **tabulOut);

	void UmaVida(int lin, int col,int **tabulIn, int **tabulOut);

	void DumpTabuleiro(int Vet[], int col,int **tabul, int pri, int ult);
	
	int Correto(int lin, int col, int **tabulIn);
	
	void Divide (int tam, int nProc, int Vet[]);

	int main(int argc, char* argv[]){
	
		//Variaveis usadas somente pelo mestre
		int c, p;
		
		//Variaveis criadas para a paralelização
		int i;
		int nProc;
		int esteProc;
		MPI_Status status;//status para Recv
		MPI_Request request;// request para Isend
		
		//Variaveis usadas pelo mestre e escravo
		int n = 32;
		int correto;
		int nPot;
		int j, k;
		int **tabulIn;
		int **tabulOut;
		
		MPI_Init(&argc, &argv);//Inicia MPI
		MPI_Comm_size(MPI_COMM_WORLD, &nProc);
		MPI_Comm_rank(MPI_COMM_WORLD, &esteProc);
		int Vet[nProc];//vetor para armazenar divisões balanceadas
		
		Divide(n, nProc, Vet);
		//for(i=0; i < nProc;i++)
			//printf("Vet[%d]=%d\n",i,Vet[i]);
		//}
		for (i = 0; i < nProc; i++){
			if (esteProc == i){

				//aloca dois tabuleiros
				tabulIn = (int**) malloc ((Vet[i]+2) * sizeof(int*));
				tabulOut = (int**) malloc ((Vet[i]+2) * sizeof(int*));
				for (j = 0; j < (Vet[i]+2); j++){
					tabulIn[j] = (int*) malloc ((n+2) * sizeof(int));	
					tabulOut[j] = (int*) malloc ((n+2) * sizeof(int));
				}
				
				//Inicializa dois tabuleiros
				//tabuleiro tabulIn contem um veleiro
				InitTabuleiroLocal(Vet[i], n, tabulIn, tabulOut);
				
				if (esteProc == 0){
					printf("***Execucao com %d Processos***\n\n",nProc);
					printf("Estado Inicial");
				}
				DumpTabuleiro(Vet, n, tabulIn, 1, n);
				
				//todas as geracoes para um veleiro
				for (k = 1; k <= (2*(n-3)); k++){

					//geracao impar
					UmaVida(Vet[i], n, tabulIn, tabulOut);
					
					//Troca Mensagens - atualiza zona fantasma
					if (esteProc != nProc-1){
						MPI_Isend(tabulOut[Vet[i]], n+2, MPI_INT, esteProc+1, esteProc, MPI_COMM_WORLD, &request);
					}
					if ( esteProc != 0){
						MPI_Recv(tabulOut[0], n+2, MPI_INT, esteProc-1, esteProc-1, MPI_COMM_WORLD, &status);
					}
					if ( esteProc != 0){
						MPI_Isend(tabulOut[1], n+2, MPI_INT, esteProc-1, esteProc+10, MPI_COMM_WORLD, &request);
					}
					if (esteProc != nProc-1){
						MPI_Recv(tabulOut[Vet[i]+1], n+2, MPI_INT, esteProc+1, esteProc+11, MPI_COMM_WORLD, &status);
					}
					if (esteProc == 0)
						printf("Geracao %d", 2*k-1);	
					DumpTabuleiro(Vet, n, tabulOut, 1, n);
					
					//geracao par
					UmaVida(Vet[i], n, tabulOut, tabulIn);
					
					//Troca Mensagens - atualiza zona fantasma
					if (esteProc != nProc-1){
						MPI_Isend(tabulIn[Vet[i]], n+2, MPI_INT, esteProc+1, esteProc, MPI_COMM_WORLD, &request);
					}
						if ( esteProc != 0){
						MPI_Recv(tabulIn[0], n+2, MPI_INT, esteProc-1, esteProc-1, MPI_COMM_WORLD, &status);
					}
					if ( esteProc != 0){
						MPI_Isend(tabulIn[1], n+2, MPI_INT, esteProc-1, esteProc+10, MPI_COMM_WORLD, &request);
					}
					if (esteProc != nProc-1){
						MPI_Recv(tabulIn[Vet[i]+1], n+2, MPI_INT, esteProc+1, esteProc+11, MPI_COMM_WORLD, &status);
					}
					if (esteProc == 0)
						printf("Geracao %d", 2*k);
					DumpTabuleiro(Vet, n, tabulIn, 1, n);
					
				}//Final for das geraçoes de um veleiro

				//imprime tempo sse resultado correto
				if (esteProc == 0){
					c=0;
					for( p = 1; p < nProc; p++){
						MPI_Recv(&correto, 1, MPI_INT, p, p, MPI_COMM_WORLD, &status);	
						if (correto == 1) 
							c++;			
					}
					if ((c == (nProc-1)) && (Correto(Vet[i], n, tabulIn)) == 1){					
						printf("Resultado Correto\n");
					}				
					else{
						printf("Resultado errado\n");
					}
				}
				else{
					correto = Correto(Vet[i], n, tabulIn);
					MPI_Send(&correto, 1, MPI_INT, 0, esteProc, MPI_COMM_WORLD);
				}			
				//destroi tabuleiros
				for (j = 0; j < (Vet[i]+2); j++){
					free(tabulIn[j]);
					free(tabulOut[j]);
				}
				free(tabulIn);
				free(tabulOut);
			}// Final If dos processadores
		}//final for dos processadores
		MPI_Finalize();
		return 0;
	}//Final Main
