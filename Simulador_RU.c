/*		Trabalho I - Sistemas Operacionais - 2015.2
	Simulador de Restaurante Universitário com conceitos de Processos
		Por: Genicleito Gonçalves (213105012)
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <semaphore.h>

//int _lugares = 10;
int _estudantes = 0;
int _ruAberto = 1;
int _qtdEstudantes;
int _refeicoes = 8000;
int _horarioFuncionamento = 100;		// HorarioFuncionamento = 100 ~= 1m40s
int _tempo;


void *estudante(void *id);	// Estudantes que chegam ao RU
void recepcionista(void *);
void catraca(void *);
void vagas(void *);

sem_t fila_espera_comprar;	// Fila para comprar fichas
sem_t fila_espera_rampa;	// Fila para a rampa de alimentação
sem_t cadeiras;	// Vende as fichas aos estudantes e verifica se ainda há refeições (OBS: TALVEZ SEJA DESNECESSÁRIO)
sem_t mutex;

int main(){
	srand((unsigned)time(NULL));
	_qtdEstudantes = rand() % (_refeicoes * 2);
	int i;
	
	pthread_t estudantes_t[_qtdEstudantes], atendente_t, catraca_t;

	sem_init(&mutex, 0, 1);
	sem_init(&fila_espera_comprar, 0, 0);
	sem_init(&fila_espera_rampa, 0, 0);
	sem_init(&cadeiras, 0, 5);

	pthread_create(&atendente_t, NULL, (void *)recepcionista, NULL);
	
	pthread_create(&catraca_t, NULL, (void *) catraca, NULL);
	
	for(i = 0; i < _qtdEstudantes; i++){
		pthread_create(&estudantes_t[i], NULL, estudante, &i);
		_tempo = rand() % 5;
		sleep(_tempo);
		_horarioFuncionamento -= _tempo;
	}
	
	/* Chamada para as Threads estudantes */
	for(i = 0; i < _qtdEstudantes; i++){
		pthread_join(estudantes_t[i], NULL);
	}
	
	//pthread_join(seguranca_t, NULL);
	pthread_join(atendente_t, NULL);
	pthread_join(catraca_t, NULL);
	
	return EXIT_SUCCESS;
}

void *estudante (void *id){
	sem_wait(&mutex);	// Região Crítica (Equivalente ao Down)
	//int iD = *((int *) id);
	fprintf(stderr, "Novo estudante chegou ao RU...\n");
	if(_refeicoes > 0){	// RU está aberto
		//_estudantes++;
		fprintf(stderr, "Mais um estudante na fila...\n");
		sem_post(&mutex);	// Sai da região crítica
		sem_post(&fila_espera_comprar);	// Estudante esta na fila para comprar (wait em recepcionista)
		//sleep(2);
	}else{
		fprintf(stderr, "Este estudante vai para aula com fome...\n");
		sem_post(&mutex);	// Sai da região crítica
	}
	//sleep(2);
	
	return NULL;
}

void recepcionista(void *v){
	while(1){
		if(_refeicoes > 0 && _estudantes < _qtdEstudantes && _horarioFuncionamento > 0){
			sem_wait(&fila_espera_comprar);
			sem_wait(&mutex);
			fprintf(stderr, "Estudante esta sendo atendido...\n");
			_estudantes++;
			sleep(2);
			_horarioFuncionamento -= 2;
			_refeicoes--;
			fprintf(stderr, "Estudante comprou a ficha... indo para a rampa...\n");
			sem_post(&mutex);
			sem_post(&fila_espera_rampa);	//	Foi para a rampa
			//fprintf(stderr, "Estudante indo para a rampa...\n");
			//sleep(3);
		}else if(_refeicoes <= 0){
			sem_wait(&mutex);
			fprintf(stderr, "==> [ COMIDA ACABOU... RU FECHOU!!! ]\n");
			//_ruAberto = 0;	// Acabou a comida, RU fechado
			sem_post(&mutex);
			break;
		}else if(_horarioFuncionamento <= 0){
			sem_wait(&mutex);
			fprintf(stderr, "==> [ EXPEDIENTE ACABOU!!! ]\n");
			_refeicoes = 0;
			sem_post(&mutex);
			break;
		}else{
			fprintf(stderr, "==> [ ESPERANDO ACABAR O EXPEDIENTE... ]\n");
			sleep(_horarioFuncionamento);
			sem_wait(&mutex);
			fprintf(stderr, "==> [ EXPEDIENTE ACABOU!!! ]\n");
			_refeicoes = 0;
			sem_post(&mutex);
			break;
		}
	}
}

void catraca (void *v){
	while(1){
		sem_wait(&mutex);
		fprintf(stderr, "===> _estudantes = %d <=====...\n", _estudantes);
		fprintf(stderr, "===> _qtdEstudantes = %d <====\n", _qtdEstudantes);
		if(_refeicoes > 0 && _estudantes < _qtdEstudantes){
			sem_post(&mutex);
			sem_wait(&fila_espera_rampa);
			sem_wait(&mutex);
			fprintf(stderr, "Estudante esta na rampa se servindo...\n");
			sleep(2);
			//_horarioFuncionamento -= 2;
			sem_post(&mutex);
			fprintf(stderr, "Estudante saiu da rampa...\n");
			fprintf(stderr, "Estudante esta indo comer...\n");
			sem_wait(&cadeiras);		// tem cadeiras disponíveis ?
			sleep(2);
			//_horarioFuncionamento -= 2;
			fprintf(stderr, "Estudante terminou de comer e foi estudar...\n");
			sem_post(&cadeiras);
		}else{
			sem_post(&mutex);
			break;
		}
	}
}
