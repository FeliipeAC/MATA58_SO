/*
 * Trabalho II - Sistemas Operacionais - 2015.2
 * Por: Genicleito Carvalho Beltrão Gonçalves (213105012)
 * Renomear vários arquivos
 * */

#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
	int i, j, processos[argc - 2];	/* O número de parametros menos o primeiro parametro que é o nome do programa chamado e
									 * o ultimo que é o prefixo	*/
	char linha[550];
	
	/* Criação de processos para gerenciar os arquivos	*/
	for (i = argc - 3, j = 1; i >= 0; --i, j++){
		processos[i] = fork();
		if (processos[i] == 0){
			
			FILE *arqOrigem, *arqDestino;
			
			if(!(arqOrigem = fopen(argv[j], "r"))){
				perror("Erro ao ler arquivo.");
				fclose(arqOrigem);
				exit(1);
			}
			
			char nome[strlen(argv[j]) + strlen(argv[argc - 1]) + 1];
			
			strcpy(nome, argv[argc - 1]);
			strcat(nome, argv[j]);
			
			/* Enquanto não chegar ao final do arquivo */
			while ((fscanf(arqOrigem, "%[^\n]%*c", linha)) != EOF){
				
				if(!(arqDestino = fopen(nome, "a"))){
					perror("Erro ao criar arquivo.");
					fclose(arqDestino);
					exit(1);
				}
				
				fprintf(arqDestino, "%s\n", linha);
				
				fclose(arqDestino);
				
			}
			
			/*	Fechando todos os arquivos abertos, ao final da leitura	*/
			fclose(arqOrigem);
				
			remove(argv[j]);	/* Cada processo remove todos os arquivos de origem	*/
			_exit(0);
		}
	}

	/* Espera todos os processos terminarem sua execução	*/
	for (i = argc - 3; i >= 0; --i){
		waitpid(processos[i], NULL, 0);
	}
	
	return 0;
}
