# The Senate Bus problem
## Grupo
* Luiza Coelho de Souza - 247257
* Pedro Henrique Peraçoli Pereira Ceccon - 247327
* Raphael Ferezin Kitahara - 244389
* Tiago Godoi Bannwart - 215386

## O problema
  O problema selecionado por nós foi o The Senate Bus problem, descrito no livro "The Little Book of Semaphores". 
  O problema consiste em passagageiros que chegam em um ponto de ônibus e esperam sua chegada. Quando o ônibus chega, os passageiros que já estavam esperando podem embarcar
  no ônibus, respeitando o limite máximo de pessoas dentro do ônibus. Qualquer pessoa que chegar enquanto o embarque estiver acontecendo, terá que esperar pelo
  próximo. Por exemplo, se a capacidade for 50 pessoas, e tiverem 100 pessoas esperando, 50 entrarão e as restantes terão que aguardar o próximo ônibus. Mesmo que existam
  menos pessoas esperando do que o limite, as pessoas que chegarem após a chegada do ônibus, não poderão embarcar nele. Se não houver nenhum passageiro esperando pelo ônibus,
  ele chegará e partirá imediatamente. A partir do enunciado apresentado, imaginamos como esta situação poderia ser aplicada à Unicamp, ou seja, algo mais próximo
  da nossa realidade, na qual os passageiros seriam os estudantes e o ônibus seria o circular.

## Nossa implementação
  Para garantir a variabilidade da implementação do nosso problema, criamos três defines:
  <br>
  ```ONIBUS_TEMPO```: Controla o tempo entre a criação de cada ônibus.
  <br>
  ```PASSAGEIROS_TEMPO```: Controla o tempo entre a criação de cada passageiro.
  <br>
  ```MAX_PASSAGEIROS```: Define o número máximo de passageiros no ônibus e na fila de embarque silmultaneamente.
  <br>
  ```MAX_ONIBUS```: Define o número de onibus que vão passar antes do fim da execução do programa. Se for colocado um número negativo o programa vai rodar indefinidamente.
  
  ### Ônibus
  Criamos uma thread para cada ônibus. Primeiramente ela bloqueia o mutex que permite a entrada de novos passageiros na fila de embarque (```pode_entrar_fila_embarque```), fazendo que novos passageiros que chegarem entrem na fila de espera. Depois, se houverem passageiros para embarcar, o ônibus libera o mutex de embarque (```pode_entrar_onibus```), se não ele parte imediatamente. Quando todos os passageiros tiverem embarcado, liberamos o mutex que permite a partida do ônibus (```todos_embarcados```). Ademais,  a entrada de passageiros na fila de embarque é liberada (```pode_entrar_fila_embarque```).
  ```c
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
  ```
  ### Passageiro
  Criamos uma thread para cada passageiro. Inicialmente, ela atualiza o número de passageiros na fila de espera. Depois, se houver espaço na fila de embarque e se não estiver bloqueada pelo mutex (```pode_entrar_fila_embarque```), o passageiro entra na mesma, decrementando o multiplex que controla a quantidade atual de passageiros nesta fila e limita a quantidade de passageiros nela (```encheu_fila_embarque```). Dessa forma, quando o número de passeiros na fila de embarque atingir a capacidade máxima do ônibus, ela é bloqueada.
  Quando o passageiro entra na fila de embarque, atualizamos a quantidade de passageiros na espera e no embarque. Se o ônibus estiver parado e aberto para o embarque, o mutex é liberado (```pode_entrar_onibus```) e os passageiros na fila de embarque começam a entrar no ônibus um a um, incrementando o multiplex que controla a quantidade atual de passageiros na mesma (```encheu_fila_embarque```). Ademais, se não houverem passageiros na fila de embarque, o mutex para a partida do ônibus é liberado (```todos_embarcados```).
  ```c
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
  ```

  ### Semáforos
  Começamos a implementar criando os semáforos, sendo eles, um mutex que permite a entrada na área de espera (```pode_entrar_fila_embarque```), um multiplex para controlar o 
  número máximo de passageiros (```encheu_fila_embarque```), um mutex para controlar o embarque no ônibus (```pode_entrar_onibus```) e outro mutex para controlar a saída do ônibus (```todos_embarcados```).
  ```c
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
  ```

  ### Implementação Principal
  Para rodar o programa criamos duas threads:
  * Uma que cria threads de passageiros até que os ônibus parem de passar (```criar_passageiro()```).
  ```c
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
  ```
  * Uma que cria threads de ônibus até que tenha passado o número máximo de ônibus (```criar_onibus()```). Ela confere se o número de ônibus que tem que passar já chegou em zero, e decrementa um dessa variável para cada ônibus que parte, se a váriavel for negativa, ela cria ônibus indefinidamente.
  ```c
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
  ```

  Na main criamos as duas threads supracitadas e esperamos elas acabarem suas excuções para que o programa termine todos os semáforos e avise o usuário que os ônibus acabaram.
  ```c
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
  ```

## Animação
Para a animação, pensamos em criar um diagrama no terminal que representasse a solução proposta para o problema de maneira visual e de fácil entendimento. Dessa forma, criamos três funções que realizam a formatação e impressão da fila de espera (```printar_espera()```), da área de embarque (```printar_embarque()```) e do status do ônibus (```printar_onibus()```).



Na função ```printar_espera()```, o número de fileiras - divididas de 15 em 15 pessoas - a serem impressas, com base no número de passageiros na fila de espera, é calculado e os passageiros são impressos no terminal:

```c
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
```

A função ```printar_embarque()```, calcula o número de fileiras da mesma maneira, mas com base no número de passageiros na fila de embarque, e imprime as informações no terminal:

```c
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
```

Para a função ```printar_onibus()```, primeiro criamos o “teto” do ônibus, depois alocamos os passageiros que embarcaram - em fileiras de 10 em 10 - e, por fim, adicionamos o “chão” do veículo. Após isso, chama-se as outras duas funções de impressão:

```c
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
```

Essas funções são chamadas a cada mudança que ocorre durante a execução do programa, para que o funcionamento multithread possa ser claramente visualizado.


## Dificuldades encontradas
### Encontrando o problema
Encontramos algumas dificuldades durante o processo. Inicialmente, para escolher o problema, nos deparamos com diversas opções e cenários diferentes, o que nos fez analisar com muita cautela qual iriamos selecionar. Após uma longa discussão, decidimos pelo problema do Senate Bus por algumas razões: a adaptabilidade do problema para a realidade dos integrantes do grupo, podendo relacionar com os Circulares da Unicamp, e a ampla gama dos conceitos de Sistemas Operacionais que poderiam ser abordados na resolução do problema.

### Implementação
Durante a implementação da resolução do problema em sí, também encontramos alguns desafios. Conseguir coordenar os semáforos, mutex, multiplex e todas as varáveis necessárias no código foi algo que nos gerou um certo trabalho para tentar resolver, visto a alta necessidade de coordenação entre os componentes. Ademais, a tradução do problema em sí para o código também foi algo complicado, visto que haviam certas condições que não estavam explicitas no enunciado e que eram necessárias para a nossa implementação. Dessa forma, a tomada de decisões sobre esses pontos também foi algo bem trabalhado pelo grupo.

### Dficuldades na animação
Por fim, durante a implementação da animação do código, encontramos algumas dificuldades tanto práticas quanto visuais. Visto que a quantidade de informações que gostariamos de transmitir na animação era ampla, coordenar o aspecto visual da animação foi um desafio, no qual houvemos que pensar cuidadosamente onda cada componente iria. Além disso, o tamanho do ônibus deveria ser adaptável, visto que, buscando uma variabilidade do código, o número máximo de passageiros era variável, ou seja, o tamanho do ônibus também deveria ser, o que se provou um desafio na implementação.

