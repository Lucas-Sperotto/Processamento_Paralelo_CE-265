#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mpi.h"

	void InitTabuleiroLocal(int lin, int col, int **tabulIn, int **tabulOut);
	
	void InitTabuleiroGlobal(int tam, int **tabulIn, int **tabulOut);

	void UmaVida(int lin, int col,int **tabulIn, int **tabulOut);
	
	int Correto(int lin, int col, int **tabulIn);
	
	void Divide (int tam, int nProc, int Vet[]);

	float wall_time(void);

	int main(int argc, char* argv[]){
		MPI_Init(&argc, &argv);//Inicia MPI
		//Variaveis usadas somente pelo mestre
		int c, p;
		float tInicial = 0.0, tFinal = 0.0;
		
		//Variaveis criadas para a paraleliza��o
		int i;
		int nProc;
		int esteProc;
		MPI_Status status;//status para Recv
		MPI_Request request;// request para Isend
		int iter;
		//Variaveis usadas pelo mestre e escravo
		int n, correto;
		int nPot;
		int nPotMin=5;
		int nPotMax=10;
		int j, k;
		int **tabulIn;
		int **tabulOut;
		
		
		MPI_Comm_size(MPI_COMM_WORLD, &nProc);
		MPI_Comm_rank(MPI_COMM_WORLD, &esteProc);
		int Vet[nProc];//vetor para armazenar divis�es balanceadas
		
		if (esteProc == 0){
			printf("***Execucao com %d Processos***\n\n",nProc);
			printf("    Tamanho          Tempo\n");
		}

		//faz para diversos tamanhos
		for (nPot = nPotMin; nPot <= nPotMax; nPot++){
			n = pow(2, nPot);
			//printf("%d\n",n);
			
			Divide(n, nProc, Vet);
			//for(i=0; i < nProc;i++)
				//printf("Vet[%d]=%d\n",i,Vet[i]);
			//}
		if ((nProc != 4) && (nProc != 6) && (nProc != 8)){
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
					//DumpTabuleiro(Vet, n, tabulIn, 1, n, n);
					//mede tempo
					if (esteProc == 0)
						tInicial = wall_time();
	
					//todas as geracoes para um veleiro
					for (k = 1; k <= (2*(n-3)); k++){

						//geracao impar
						UmaVida(Vet[i], n, tabulIn, tabulOut);
						//DumpTabuleiro(Vet, n, tabulOut, 1, n, n);
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
						
						//geracao par
						UmaVida(Vet[i], n, tabulOut, tabulIn);
						//DumpTabuleiro(Vet, n, tabulIn, 1, n, n);
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
					}//Final for das gera�oes de um veleiro

					// mede tempo
					if (esteProc == 0)	
						tFinal = wall_time();
			
					//imprime tempo sse resultado correto
					if (esteProc == 0){
						c=0;
						for( p = 1; p < nProc; p++){
							MPI_Recv(&correto, 1, MPI_INT, p, p, MPI_COMM_WORLD, &status);	
							if (correto == 1) 
								c++;			
						}
						if ((c == (nProc-1)) && (Correto(Vet[i], n, tabulIn)) == 1){					
							printf("      %4.d      %.14f \n", n, tFinal-tInicial);
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
		}//final do if
		else{
			iter = n;
			n = n/2;
			for (i = 0; i < nProc; i++){
			Vet[i] = Vet[i]*2;}
			for (i = 0; i < nProc; i++){
				if (esteProc == i){
					//iter = n;
			    	//Vet[i] = Vet[i]*2;
					//n = n/2;
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
					//DumpTabuleiro(Vet, n, iter, tabulIn, 1, n);
					//MPI_Barrier(MPI_COMM_WORLD);
					//if (esteProc == 7){	
						//tFinal = wall_time();
						//for( p = 1; p < Vet[i]+1; p++){
						//	for( j = 1; j < n+1; j++){
						//		printf("%d", tabulIn[p][j]);
						//	}
						//	printf("\n");
						//}printf("\n\n");
					//}
					//mede tempo
					if (esteProc == 0){
						tInicial = wall_time();
						}
					//todas as geracoes para um veleiro
					for (k = 1; k <= (2*(iter-3)); k++){

						//geracao impar
						//MPI_Barrier(MPI_COMM_WORLD);
						UmaVida(Vet[i], n, tabulIn, tabulOut);
						//DumpTabuleiro(Vet, n, iter, tabulOut, 1, n);
						//MPI_Barrier(MPI_COMM_WORLD);
						//if (esteProc == 7){	
						//tFinal = wall_time();
						//for( p = 1; p < Vet[i]+1; p++){
						//	for( j = 1; j < n+1; j++){
							//	printf("%d", tabulIn[p][j]);
						//	}
						//	printf("\n");
						//}printf("\n\n");
					//}
						//MPI_Barrier(MPI_COMM_WORLD);
						if (esteProc%2 ==0){
							//Troca Mensagens - atualiza zona fantasma
							for(p = 1; p < (Vet[i]+1); p++){
							MPI_Send(&tabulOut[p][n], 1, MPI_INT, esteProc+1, p+esteProc+100, MPI_COMM_WORLD);
							}
							for(p = 1; p < (Vet[i]+1); p++){
							MPI_Recv(&tabulOut[p][n+1], 1, MPI_INT, esteProc+1, p+esteProc+201, MPI_COMM_WORLD, &status);
							}
							if (esteProc != (nProc-2)){
								MPI_Isend(&tabulOut[Vet[i]][1], n, MPI_INT, esteProc+2, esteProc+1000, MPI_COMM_WORLD, &request);
								MPI_Isend(&tabulOut[Vet[i]][n], 1, MPI_INT, esteProc+3, esteProc+30, MPI_COMM_WORLD, &request);
							}
							if (esteProc != 0){
								MPI_Recv(&tabulOut[0][1], n, MPI_INT, esteProc-2, esteProc+998, MPI_COMM_WORLD, &status);
								MPI_Recv(&tabulOut[0][n+1], 1, MPI_INT, esteProc-1, esteProc+49, MPI_COMM_WORLD, &status);
							}
							if (esteProc != 0){
								MPI_Isend(&tabulOut[1][1], n, MPI_INT, esteProc-2, esteProc+2000, MPI_COMM_WORLD, &request);
								MPI_Isend(&tabulOut[1][n], 1, MPI_INT, esteProc-1, esteProc+80, MPI_COMM_WORLD, &request);
							}
							if (esteProc != (nProc-2)){
								MPI_Recv(&tabulOut[Vet[i]+1][1], n, MPI_INT, esteProc+2, esteProc+2002, MPI_COMM_WORLD, &status);
								MPI_Recv(&tabulOut[Vet[i]+1][n+1], 1, MPI_INT, esteProc+3, esteProc+63, MPI_COMM_WORLD, &status);
							}
						}	
						else{
							for(p = 1; p < (Vet[i]+1); p++){
							MPI_Recv(&tabulOut[p][0], 1, MPI_INT, esteProc-1, p+esteProc+99, MPI_COMM_WORLD, &status);
							}
							for(p = 1; p < (Vet[i]+1); p++){
							MPI_Send(&tabulOut[p][1], 1, MPI_INT, esteProc-1, p+esteProc+200, MPI_COMM_WORLD);
							}
							if (esteProc != nProc-1){
								MPI_Isend(&tabulOut[Vet[i]][1], n, MPI_INT, esteProc+2, esteProc, MPI_COMM_WORLD, &request);
								MPI_Isend(&tabulOut[Vet[i]][1], 1, MPI_INT, esteProc+1, esteProc+50, MPI_COMM_WORLD, &request);
							}
							if (esteProc != 1){
								MPI_Recv(&tabulOut[0][1], n, MPI_INT, esteProc-2, esteProc-2, MPI_COMM_WORLD, &status);
								MPI_Recv(&tabulOut[0][0], 1, MPI_INT, esteProc-3, esteProc+27, MPI_COMM_WORLD, &status);
							}
							if (esteProc != 1){
								MPI_Isend(&tabulOut[1][1], n, MPI_INT, esteProc-2, esteProc+10, MPI_COMM_WORLD, &request);
								MPI_Isend(&tabulOut[1][1], 1, MPI_INT, esteProc-3, esteProc+60, MPI_COMM_WORLD, &request);
							}
							if (esteProc != nProc-1){
								MPI_Recv(&tabulOut[Vet[i]+1][1], n, MPI_INT, esteProc+2, esteProc+12, MPI_COMM_WORLD, &status);
								MPI_Recv(&tabulOut[Vet[i]+1][0], 1, MPI_INT, esteProc+1, esteProc+81, MPI_COMM_WORLD, &status);
							}
						}
						//geracao par
						//MPI_Barrier(MPI_COMM_WORLD);
						UmaVida(Vet[i], n, tabulOut, tabulIn);
						//DumpTabuleiro(Vet, n, iter, tabulIn, 1, n);
						//MPI_Barrier(MPI_COMM_WORLD);
						//if (esteProc == 7){	
						//tFinal = wall_time();
						//for( p = 1; p < Vet[i]+1; p++){
						//	for( j = 1; j < n+1; j++){
						//		printf("%d", tabulIn[p][j]);
						//	}
							//printf("\n");
						//}printf("\n\n");
					//}
						//MPI_Barrier(MPI_COMM_WORLD);
					
						if (esteProc%2 ==0){
							for(p = 1; p < (Vet[i]+1); p++){
							MPI_Send(&tabulIn[p][n], 1, MPI_INT, esteProc+1, p+esteProc+100, MPI_COMM_WORLD);
							}
							for(p = 1; p < (Vet[i]+1); p++){
							MPI_Recv(&tabulIn[p][n+1], 1, MPI_INT, esteProc+1, p+esteProc+201, MPI_COMM_WORLD, &status);
							}
							if (esteProc != (nProc-2)){
								MPI_Isend(&tabulIn[Vet[i]][1], n, MPI_INT, esteProc+2, esteProc+1000, MPI_COMM_WORLD, &request);
								MPI_Isend(&tabulIn[Vet[i]][n], 1, MPI_INT, esteProc+3, esteProc+30, MPI_COMM_WORLD, &request);
							}
							if (esteProc != 0){
								MPI_Recv(&tabulIn[0][1], n, MPI_INT, esteProc-2, esteProc+998, MPI_COMM_WORLD, &status);
								MPI_Recv(&tabulIn[0][n+1], 1, MPI_INT, esteProc-1, esteProc+49, MPI_COMM_WORLD, &status);

							}
							if (esteProc != 0){
								MPI_Isend(&tabulIn[1][1], n, MPI_INT, esteProc-2, esteProc+2000, MPI_COMM_WORLD, &request);
								MPI_Isend(&tabulIn[1][n], 1, MPI_INT, esteProc-1, esteProc+80, MPI_COMM_WORLD, &request);
							}
							if (esteProc != (nProc-2)){
								MPI_Recv(&tabulIn[Vet[i]+1][1], n, MPI_INT, esteProc+2, esteProc+2002, MPI_COMM_WORLD, &status);
								MPI_Recv(&tabulIn[Vet[i]+1][n+1], 1, MPI_INT, esteProc+3, esteProc+63, MPI_COMM_WORLD, &status);
							}
						}	
						else{
							for(p = 1; p < (Vet[i]+1); p++){
							MPI_Recv(&tabulIn[p][0], 1, MPI_INT, esteProc-1, p+esteProc+99, MPI_COMM_WORLD, &status);
							}
							for(p = 1; p < (Vet[i]+1); p++){
							MPI_Send(&tabulIn[p][1], 1, MPI_INT, esteProc-1, p+esteProc+200, MPI_COMM_WORLD);
							}
							if (esteProc != nProc-1){
								MPI_Isend(&tabulIn[Vet[i]][1], n, MPI_INT, esteProc+2, esteProc, MPI_COMM_WORLD, &request);
								MPI_Isend(&tabulIn[Vet[i]][1], 1, MPI_INT, esteProc+1, esteProc+50, MPI_COMM_WORLD, &request);
							}
							if (esteProc != 1){
								MPI_Recv(&tabulIn[0][1], n, MPI_INT, esteProc-2, esteProc-2, MPI_COMM_WORLD, &status);
								MPI_Recv(&tabulIn[0][0], 1, MPI_INT, esteProc-3, esteProc+27, MPI_COMM_WORLD, &status);
							}
							if (esteProc != 1){
								MPI_Isend(&tabulIn[1][1], n, MPI_INT, esteProc-2, esteProc+10, MPI_COMM_WORLD, &request);
								MPI_Isend(&tabulIn[1][1], 1, MPI_INT, esteProc-3, esteProc+60, MPI_COMM_WORLD, &request);
							}
							if (esteProc != nProc-1){
								MPI_Recv(&tabulIn[Vet[i]+1][1], n, MPI_INT, esteProc+2, esteProc+12, MPI_COMM_WORLD, &status);
								MPI_Recv(&tabulIn[Vet[i]+1][0], 1, MPI_INT, esteProc+1, esteProc+81, MPI_COMM_WORLD, &status);
							}
						}
					}//Final for das gera�oes de um veleiro
					// mede tempo
					if (esteProc == 0){	
						tFinal = wall_time();
					
					}
					//imprime tempo sse resultado correto
					if (esteProc == 0){
						c=0;
						for( p = 1; p < nProc; p++){
							MPI_Recv(&correto, 1, MPI_INT, p, p, MPI_COMM_WORLD, &status);	
							if (correto == 1) 
								c++;			
						}
						if ((c == (nProc-1)) && (Correto(Vet[i], n, tabulIn)) == 1){					
							printf("      %4.d      %.14f \n", iter, tFinal-tInicial);
						}				
						else{
							printf("Resultado errado\n");
						}
					}
					else{
						correto = Correto(Vet[i], n, tabulIn);
						MPI_Send(&correto, 1, MPI_INT, 0, esteProc, MPI_COMM_WORLD);
						//printf("Proc %d = %d\n", esteProc, correto);
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
		}//Final do ELSE
		}//final for geral
		MPI_Finalize();
		return 0;
	}//Final Main
