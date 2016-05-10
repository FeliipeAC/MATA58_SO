/*		Trabalho I - Sistemas Operacionais - 2015.2
	Simulador de Restaurante Universitário com conceitos de Processos/Threads
		Por: Genicleito Carvalho Beltrão Gonçalves (213105012)
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <semaphore.h>

int _estudantes = 0;	/* Quantidade de estudantes que chegaram ao Restaurante Universitário e conseguiram se alimentar	*/
int _qtdEstudantes;		/* Quantidade total de estudantes que chegaram ao RU (Definido aleatoriamente)	*/
int _refeicoes = 20;	/* Quantidade de refeições que serão servidas neste expediente	*/
/* Horário total de funcionamento do RU (Duração do Expediente)	*/
int _horarioFuncionamento = 50;		/* Com _horarioFuncionamento igual a 100, expediente dura aproximadamente 1m40s	*/
int _tempo;		/* Relação de tempo com o _horarioFuncionamento definido logo abaixo	*/
int _filaRampa;	/* Contador de pessoas na fila para a rampa de alimentação	*/
time_t t_inicio, t_fim;		/* Variáveis para registro do tempo de execução do programa	*/


void estudante(void *);	/* Função para controle dos estudantes que chegam ao Restaurante Universitário	*/
void recepcionista(void *);	/* Função para controle de venda das fichas para os estudantes	*/
void catraca(void *);		/* Função para controle do acesso e saída de estudantes na rampa de alimentação	*/

sem_t fila_espera_comprar;	/* Semáforo para controle da fila para comprar fichas	*/
sem_t fila_espera_rampa;	/* Semáforo para controle da fila para a rampa de alimentação	*/
sem_t cadeiras;				/* Semáforo para controle do acesso às cadeiras do Restaurante Universitário	*/
sem_t mutex;				/* Semáforo binário para regiões críticas	*/

int main(){
	srand((unsigned)time(NULL));
	
	/* A quantidade total de estudantes que chegarão ao RU pode
	 * ser de até duas vezes o número de refeições
	 * servidas neste expediente (Será gerado aleatóriamente)	*/
	_qtdEstudantes = rand() % (_refeicoes * 2);
	int i;
	
	/* Threads para controle das funções de chegada de estudantes, atendimento aos estudantes
	 * e catraca para a rampa de alimentação. É criado um vetor de threads com a quantidade total
	 * aletória	de estudantes que chegarão ao RU neste expediente	*/
	pthread_t estudantes_t[_qtdEstudantes], atendente_t, catraca_t;

	/* Inicialização dos Semáforos	*/
	sem_init(&mutex, 0, 1);
	sem_init(&fila_espera_comprar, 0, 0);
	sem_init(&fila_espera_rampa, 0, 0);
	sem_init(&cadeiras, 0, 15);

	t_inicio = time(NULL);

	/* Chamada à thread atendente_t	*/
	pthread_create(&atendente_t, NULL, (void *)recepcionista, NULL);
	/*	Chamada à thread catraca_t	*/
	pthread_create(&catraca_t, NULL, (void *) catraca, NULL);
	
	/* Chamadas do vetor de threads para as várias threads de estudantes que chegarão ao RU neste expediente	*/
	for(i = 0; i < _qtdEstudantes; i++){
		pthread_create(&estudantes_t[i], NULL, (void *) estudante, NULL);
		_tempo = rand() % 3;
		sleep(_tempo);
		_horarioFuncionamento -= _tempo;
	}
	
	/* Espera pela finalização de todas as threads de estudantes */
	for(i = 0; i < _qtdEstudantes; i++){
		pthread_join(estudantes_t[i], NULL);
	}
	
	pthread_join(atendente_t, NULL);	/* Espera pela finalização da thread responsável pela venda das fichas	*/
	pthread_join(catraca_t, NULL);		/* Espera pela finalização da thread responsável pelo gerenciamento da rampa de alimentação	*/
	
	t_fim = time(NULL);
	
	/* Levantamento e apresentação das informações colhidas durante o funcionamento do simulador	*/
	fprintf(stderr, "==> O simulador funcionou por: %.2f segundos\n", difftime(t_fim, t_inicio));
	fprintf(stderr, "==> Quantidade de estudantes que foram ate o RU: %d\n", _qtdEstudantes);
	fprintf(stderr, "==> Quantidade de estudantes que se alimentaram: %d\n", _estudantes);
	fprintf(stderr, "==> Quantidade de estudantes que nao se alimentaram: %d\n", _qtdEstudantes - _estudantes);
	
	return EXIT_SUCCESS;
}

/* Função para controle dos estudantes que chegam ao Restaurante Universitário.
 * Recebe um parametro void de valor NULL passsado na chamada e não retorna nada.	*/
void estudante (void *v){
	sem_wait(&mutex);	/* Entra na Região Crítica */
	fprintf(stderr, "Novo estudante chegou ao RU...\n");
	if(_refeicoes > 0){	/* RU ainda esta aberto pois tem refeições para serem vendidas	*/
		fprintf(stderr, "Mais um estudante na fila...\n");
		sem_post(&mutex);	/*	Sai da região crítica	*/
		sem_post(&fila_espera_comprar);	/* Estudante foi para a fila de compra	*/
	}else{	/*	Estudante chega ao RU e percebe um aviso de que a comida acabou	*/
		fprintf(stderr, "[RU FECHADO] Este estudante vai para aula com fome...\n");
		sem_post(&mutex);	/*	Sai da região crítica	*/
	}
}

/* Função para controle de venda das fichas para os estudantes.
 * Recebe um parametro void de valor NULL passsado na chamada e não retorna nada.	*/
void recepcionista(void *v){
	while(1){
		/* Caso ainda tenham refeições e ainda esteja no horário de funcionamento do expediente, a atendente atende.	*/
		if(_refeicoes > 0 && _estudantes < _qtdEstudantes && _horarioFuncionamento > 0){
			sem_wait(&fila_espera_comprar);		/* Estudante saiu da fila de compra	*/
			sem_wait(&mutex);
			fprintf(stderr, "Estudante esta sendo atendido...\n");
			_estudantes++;
			sleep(2);
			_horarioFuncionamento -= 2;		/* Opa, a atendente gastou um tempinho atendendo um estudante, vamos descontar esse tempo do horário de funcionamento total	*/
			_refeicoes--;
			fprintf(stderr, "Estudante comprou a ficha... indo para a rampa...\n");
			sem_post(&fila_espera_rampa);	/* Pessoal da rampa... este estudante esta indo ai se servir	*/
			_filaRampa++;			/* Contador de pessoas na fila da rampa	é incrementado	*/
			sem_post(&mutex);
		}else if(_refeicoes <= 0){	/* Ops, parece que acabou a comida por hoje	*/
			sem_wait(&mutex);
			fprintf(stderr, "==> [ COMIDA ACABOU... RU FECHOU!!! ]\n");
			if(_qtdEstudantes > _refeicoes){	/* Infelizmente mesmo quem estava na fila terá que voltar pra aula com fome	:( */
				fprintf(stderr, "Alguns estudantes podem ter saido da fila porque a comida acabou... \n");
			}
			sem_post(&mutex);
			break;
		}else if(_horarioFuncionamento <= 0){	/* Ebaa, terminamos nosso expediente por hoje	*/
			sem_wait(&mutex);
			fprintf(stderr, "==> [ EXPEDIENTE ACABOU!!! ]\n");
			if(_qtdEstudantes > _refeicoes){	/* Infelizmente mesmo quem estava na fila terá que voltar pra aula com fome	:( */
				fprintf(stderr, "Alguns estudantes podem ter saido da fila porque a comida acabou... \n");
			}
			_refeicoes = 0;
			sem_post(&mutex);
			
			while(_filaRampa > 0){}		/* Espera bloqueante: Esperar os estudantes comerem	*/
			
			break;
		}else{	/* Parece que ninguem mais vem comer hoje... vamos jogar dominó e esperar acabar o expediente	*/
			fprintf(stderr, "==> [ ESPERANDO ACABAR O EXPEDIENTE... ]\n");
			sleep(_horarioFuncionamento);	/* Jogando dominó	*/
			sem_wait(&mutex);
			fprintf(stderr, "==> [ EXPEDIENTE ACABOU!!! ]\n");
			_refeicoes = 0;
			sem_post(&mutex);
			
			while(_filaRampa > 0){}		/* Espera bloqueante: Esperar os estudantes comerem	*/
			
			sem_wait(&mutex);
			_horarioFuncionamento = 0;
			sem_post(&fila_espera_rampa);	/* Ainda há pessoas na rampa de alimentação	*/
			sem_post(&mutex);
			break;
		}
	}
}

/* Função para controle do acesso e saída de estudantes na rampa de alimentação.
 * Recebe um parametro void de valor NULL passsado na chamada e não retorna nada.	*/
void catraca (void *v){
	while(1){
		sem_wait(&mutex);
		/* Temos pessoas na rampa de alimentação ou o RU ainda está aberto	*/
		if(_filaRampa > 0 || _refeicoes > 0){
			sem_post(&mutex);
			sem_wait(&fila_espera_rampa);	/* Estudante sai da fila da rampa e começa a se servir	*/
			
			if(_filaRampa == 0 && _refeicoes == 0)	/* Caso o último estudante tenha saído da rampa, vamos finalizar tudo	*/
				break;
				
			sem_wait(&mutex);
			_filaRampa--;
			fprintf(stderr, "Estudante esta na rampa se servindo...\n");
			sleep(2);
			sem_post(&mutex);
			fprintf(stderr, "Estudante saiu da rampa...\n");
			fprintf(stderr, "Estudante esta indo comer...\n");
			sem_wait(&cadeiras);		/* Tem cadeiras disponíveis ? Caso não tenha... espera (dorme)	*/
			sleep(1);
			fprintf(stderr, "Estudante terminou de comer e foi estudar...\n");
			sem_post(&cadeiras);
		}else{
			sem_post(&mutex);
			break;
		}
	}
}
