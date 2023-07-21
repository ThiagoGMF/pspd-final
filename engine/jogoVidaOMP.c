#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 1234
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

#define ind2d(i, j) (i) * (tam + 2) + j
#define POWMIN 3
#define POWMAX 10

typedef struct server_message
{
    int powmin;
    int powmax;
}server_message;

double wall_time(void)
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);
    return (tv.tv_sec + tv.tv_usec / 1000000.0);
} /* fim-wall_time */

void UmaVida(int *tabulIn, int *tabulOut, int tam)
{
    int i, j, vizviv;

    #pragma omp parallel for private(i, j, vizviv) shared(tabulIn, tabulOut)
    for (i = 1; i <= tam; i++)
    {
        for (j = 1; j <= tam; j++)
        {
            vizviv = tabulIn[ind2d(i - 1, j - 1)] + tabulIn[ind2d(i - 1, j)] +
                     tabulIn[ind2d(i - 1, j + 1)] + tabulIn[ind2d(i, j - 1)] +
                     tabulIn[ind2d(i, j + 1)] + tabulIn[ind2d(i + 1, j - 1)] +
                     tabulIn[ind2d(i + 1, j)] + tabulIn[ind2d(i + 1, j + 1)];
            if (tabulIn[ind2d(i, j)] && vizviv < 2)
                tabulOut[ind2d(i, j)] = 0;
            else if (tabulIn[ind2d(i, j)] && vizviv > 3)
                tabulOut[ind2d(i, j)] = 0;
            else if (!tabulIn[ind2d(i, j)] && vizviv == 3)
                tabulOut[ind2d(i, j)] = 1;
            else
                tabulOut[ind2d(i, j)] = tabulIn[ind2d(i, j)];
        } /* fim-for */
    } /* fim-for */
} /* fim-UmaVida */

void DumpTabul(int *tabul, int tam, int first, int last, char *msg)
{
    int i, ij;

    printf("%s; Dump posicoes [%d:%d, %d:%d] de tabuleiro %d x %d\n",
           msg, first, last, first, last, tam, tam);
    for (i = first; i <= last; i++)
        printf("=");
    printf("=\n");
    for (i = ind2d(first, 0); i <= ind2d(last, 0); i += ind2d(1, 0))
    {
        for (ij = i + first; ij <= i + last; ij++)
            printf("%c", tabul[ij] ? 'X' : '.');
        printf("\n");
    }
    for (i = first; i <= last; i++)
        printf("=");
    printf("=\n");
} /* fim-DumpTabul */

void InitTabul(int *tabulIn, int *tabulOut, int tam)
{
    int ij;

    for (ij = 0; ij < (tam + 2) * (tam + 2); ij++)
    {
        tabulIn[ij] = 0;
        tabulOut[ij] = 0;
    } /* fim-for */

    tabulIn[ind2d(1, 2)] = 1;
    tabulIn[ind2d(2, 3)] = 1;
    tabulIn[ind2d(3, 1)] = 1;
    tabulIn[ind2d(3, 2)] = 1;
    tabulIn[ind2d(3, 3)] = 1;
} /* fim-InitTabul */

int Correto(int *tabul, int tam)
{
    int ij, cnt;

    cnt = 0;
    for (ij = 0; ij < (tam + 2) * (tam + 2); ij++)
        cnt = cnt + tabul[ij];
    return (cnt == 5 && tabul[ind2d(tam - 2, tam - 1)] &&
            tabul[ind2d(tam - 1, tam)] && tabul[ind2d(tam, tam - 2)] &&
            tabul[ind2d(tam, tam - 1)] && tabul[ind2d(tam, tam)]);
} /* fim-Correto */

void solveProblem(int powmin, int powmax)
{
    int pow;
    int i, tam, *tabulIn, *tabulOut;
    char msg[9];
    double t0, t1, t2, t3;

    // para todos os tamanhos do tabuleiro

    for (pow = powmin; pow <= powmax; pow++)
    {
        tam = 1 << pow;
        // aloca e inicializa tabuleiros
        t0 = wall_time();
        tabulIn = (int *)malloc((tam + 2) * (tam + 2) * sizeof(int));
        tabulOut = (int *)malloc((tam + 2) * (tam + 2) * sizeof(int));
        InitTabul(tabulIn, tabulOut, tam);
        t1 = wall_time();

        for (i = 0; i < 2 * (tam - 3); i++)
        {
            UmaVida(tabulIn, tabulOut, tam);
            UmaVida(tabulOut, tabulIn, tam);
        } /* fim-for */

        t2 = wall_time();

        if (Correto(tabulIn, tam))
            printf("**RESULTADO CORRETO**\n");
        else
            printf("**RESULTADO ERRADO**\n");

        t3 = wall_time();
        printf("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f, tot=%7.7f \n",
               tam, t1 - t0, t2 - t1, t3 - t2, t3 - t0);
        free(tabulIn);
        free(tabulOut);
    }
} 

// Função para tratar a conexão de cada cliente em uma thread separada
void *handle_client(void *arg) {
    printf("Novo cliente conectado\n");
    
    int client_socket = *((int *)arg);
    char buffer[1024] = {0};
    char *message = "Mensagem do servidor";

    // Enviar mensagem de boas-vindas ao cliente
    // send(client_socket, message, strlen(message), 0);

    // Receber e processar dados do cliente
    while (1) {
        int valread = read(client_socket, buffer, 1024);
        if (valread <= 0) {
            // Cliente desconectado ou erro na leitura
            break;
        }
        printf("Mensagem recebida do cliente: %s\n", buffer);
        // Lógica de processamento da mensagem, se necessário
        // transformar em inteiro
        // chamar a funcao solveProblem com os parametros corretos
        // retornar pro cliente ? salvar no servidor?
    }

    // Fechar o socket do cliente e terminar a thread
    close(client_socket);
    printf("Cliente desconectado\n");
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *message = "Servidor TCP/IP em C - Ola, cliente!";

    // Criando o socket servidor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Erro ao criar o socket");
        exit(EXIT_FAILURE);
    }

    // Configurando opções do socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Erro ao configurar opções do socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Vinculando o socket à porta e endereço
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Erro ao vincular o socket");
        exit(EXIT_FAILURE);
    }

    // Escutando por conexões
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Erro ao escutar");
        exit(EXIT_FAILURE);
    }

    printf("Servidor escutando na porta %d...\n", PORT);

    // Aceitando conexões de clientes
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Erro ao aceitar a conexão");
        exit(EXIT_FAILURE);
    }

    // Enviando mensagem para o cliente
    send(new_socket, message, strlen(message), 0);
    printf("Mensagem enviada para o cliente\n");

    // Lendo a mensagem do cliente
    valread = read(new_socket, buffer, BUFFER_SIZE);
    printf("Mensagem do cliente: %s\n", buffer);

    // Fechando os sockets
    close(new_socket);
    close(server_fd);

    return 0;
}



