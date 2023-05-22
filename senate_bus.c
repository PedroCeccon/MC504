#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <fcntl.h>

#define ONIBUS_TEMPO 10                     //Tempo entre a partida de um onibus e a chegada do proximo
#define PASSAGEIROS_TEMPO random() % 7      //Tempo entre a chegada de um passageiro e a chegada do proximo
#define MAX_PASSAGEIROS 10                  //Numero maximo de passageiros no onibus (e na fila de embarque)
#define MAX_ONIBUS -1                       //Numero de onibus antes que o programa encerre sua execucao sozinho,
                                            //coloque um numero negativo para que ele rode indefinidamente

sem_t *pode_entrar_fila_embarque;           //Semaforo para controlar a entrada de passageiros na fila de embarque
sem_t *encheu_fila_embarque;                //Semaforo para limitar o numero maximo de passageiros na fila de embarque (no onibus)
sem_t *pode_entrar_onibus;                  //Semaforo para controlar a entrada de passageiros no onibus
sem_t *todos_embarcados;                    //Semaforo para avisar que todos os passageiros embarcaram, liberando a partida do onibus

int controle_onibus = MAX_ONIBUS;
int onibus_chegou = 0;
int passageiros_no_onibus = 0;
int passageiros_na_espera = 0;
int passageiros_na_fila_embarque = 0;

//Imprime a fila de espera no terminal 
void printar_espera() {
    int filas = passageiros_na_espera / 15;

    printf("Fila de espera:\n");
    for (int i=0; i <= filas; i++) {
        for(int j = 0; j < 15; j++){
            if(i*15+j < passageiros_na_espera) {
                printf("o ");
            }
        }
        printf("\n");
    }
    printf("\nxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n\n");
}

//Imprime a fila de embarque no terminal
void printar_embarque() {
    int filas = passageiros_na_fila_embarque / 15;

    printf("Fila de embarque:\n");

    for (int i=0; i <= filas; i++) {
         for(int j = 0; j < 15; j++){
             if(i*15+j < passageiros_na_fila_embarque) {
                 printf("o ");
             }
         }

         printf("\n");
    }
}

//Imprime o onibus no terminal, se o onibus estiver no ponto, (automaticamente imprime as filas de espera e embarque junto)
void printar_onibus() {
    int filas = MAX_PASSAGEIROS / 10;
    if(onibus_chegou){
        //Criando teto
        printf(" ");
        for (int i = 0; i < 20; i++) {
            printf("_");
        }
        printf(" \n");
        printf("/");
        for (int i = 0; i < 20; i++) {
            printf(" ");
        }
        printf("\\\n");

        //Criando as filiras de passageiros
        int passageiros_impressos = 0;
        for (int i = 0; i <= filas; i++){
            int passageiros_na_fila = 0;
            printf("| ");
            for (int j = 0; j < 10; j++) {
                if (passageiros_impressos < passageiros_no_onibus) {
                    printf("o ");
                    passageiros_impressos++;
                    passageiros_na_fila++;
                }
            }
            for (int j=0; j < 10 - passageiros_na_fila; j++) {
                printf("  ");
            }
            printf("|\n");
        }

        //Criando o chão
        printf("\\");
        for (int i = 0; i < 4; i++) {
            printf("_");
        }
        printf("/@\\");
        for (int i = 0; i < 8; i++) {
            printf("_");
        }
        printf("/@\\");
        for (int i = 0; i < 3; i++) {
            printf("_");
        }
        printf("/\n");
    }

    printar_embarque();
    printar_espera();
}

//Thread do onibus
void *onibus() {
    //Bloqueia a entrada de passageiros na fila de embarque
    sem_wait(pode_entrar_fila_embarque);

    //Espera o embarque de todos os passageiros
    if (passageiros_na_fila_embarque > 0) {
        sleep(1);
        printf("Onibus chegou.\n");
        onibus_chegou = 1;
        printar_onibus();
        sem_post(pode_entrar_onibus);
        sem_wait(todos_embarcados);
    }

    //Sai do ponto
    printf("Onibus: Partindo com %d passageiros. \n\n", passageiros_no_onibus);
    onibus_chegou = 0;
    passageiros_no_onibus = 0;
    sleep(1);

    //Libera a entrada de passageiros na fila de embarque
    sem_post(pode_entrar_fila_embarque);

    return NULL;
}

//Thread de passageiro
void *passageiro() {

    //Entrando na fila de espera
    passageiros_na_espera += 1;
    printar_onibus();

    //Entrando na fila de embarque
    sem_wait(encheu_fila_embarque);
    sem_wait(pode_entrar_fila_embarque);
    passageiros_na_fila_embarque += 1;
    passageiros_na_espera -= 1;
    printar_onibus();
    sleep(1);

    //Entrando no onibus
    sem_post(pode_entrar_fila_embarque);
    sem_wait(pode_entrar_onibus);
    sem_post(encheu_fila_embarque);
    passageiros_na_fila_embarque-=1;
    passageiros_no_onibus += 1;
    printar_onibus();
    sleep(1);

    //Saindo do onibus
    if (passageiros_na_fila_embarque == 0) {
        sem_post(todos_embarcados);
    } else {
        sem_post(pode_entrar_onibus);
    }

    return NULL;
}

//Thread de criação de passageiros
void *criar_passageiro() {
    while(controle_onibus != 0){
        // A cada alguns segundos (PASSAGEIROS_TEMPO) cria uma nova thread de passageiro
        pthread_t passageiro_th;
        sleep(PASSAGEIROS_TEMPO);
        pthread_create(&passageiro_th, NULL, passageiro, NULL);
    }
    return NULL;
}

//Thread de criação de ônibus
void *criar_onibus() {
    pthread_t onibus_th;

    while (controle_onibus != 0) {
        // A cada alguns segundos (ONIBUS_TEMPO) cria uma nova thread de onibus
        sleep(ONIBUS_TEMPO);
        pthread_create(&onibus_th, NULL, onibus, NULL);
        pthread_join(onibus_th, NULL);
        controle_onibus = (MAX_ONIBUS < 0) ? -1 : controle_onibus - 1;
    }
    return NULL;
}


int main() {
    pthread_t criar_onibus_th, criar_passageiro_th;

    //Garantir que o semáfaros não está sendo usado por outro processo
    sem_unlink("pode_entrar_fila_embarque");
    sem_unlink("encheu_fila_embarque");
    sem_unlink("pode_entrar_onibus");
    sem_unlink("todos_embarcados");

    //Criar o semáfaros
    pode_entrar_fila_embarque = sem_open("pode_entrar_fila_embarque", O_CREAT, 0, 1);
    encheu_fila_embarque = sem_open("encheu_fila_embarque", O_CREAT, 0, MAX_PASSAGEIROS);
    pode_entrar_onibus = sem_open("pode_entrar_onibus", O_CREAT, 0, 0);
    todos_embarcados = sem_open("todos_embarcados", O_CREAT, 0, 0);

    //Cria threads que geram as threads de onibus e passageiros
    pthread_create(&criar_passageiro_th, NULL, criar_passageiro, NULL);
    pthread_create(&criar_onibus_th, NULL, criar_onibus, NULL);

    pthread_join(criar_passageiro_th, NULL);
    pthread_join(criar_onibus_th, NULL);

    //Fecha os semafaros
    sem_close(pode_entrar_fila_embarque);
    sem_close(encheu_fila_embarque);
    sem_close(pode_entrar_onibus);
    sem_close(todos_embarcados);
    
    printf("Acabaram os onibus por hoje.\n");

    return 0;
}