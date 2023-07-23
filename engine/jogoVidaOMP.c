#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <uuid/uuid.h>
#include <curl/curl.h>

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

void generate_guid(char* guid_str) {
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, guid_str);
}

int save_elastic_data(int pow, double time, int status) {
    CURL *curl;
    CURLcode res;
    char data[150];
    const char *url_template = "http://elasticsearch-service:9200/jogovida/_doc/%s";
    char full_url[200];

    char guid_str[37]; // Room for the null-terminator
    generate_guid(guid_str);

    // Format the URL with the given GUID
    sprintf(full_url, url_template, guid_str);

    // Format the JSON data
    sprintf(data, "{\"mode\": \"OpenMP\", \"potency\": \"%d\", \"time\": \"%7.7f\", \"status\": \"%d\"}", pow, time, status);

    printf("elastic flow iniciado\n");

    // Inicialização do libcurl
    curl = curl_easy_init();
    if (curl) {
        // Configuração da requisição PUT
        curl_easy_setopt(curl, CURLOPT_URL, full_url);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        // Lista de cabeçalhos
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Define o modo verbose para imprimir detalhes da requisição no console
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        printf("vai enviar requisição\n");
        // Realiza a requisição HTTP
        res = curl_easy_perform(curl);
        printf("enviou requisição\n");

        // Libera a lista de cabeçalhos
        curl_slist_free_all(headers);

        // Verifica se houve algum erro
        if (res != CURLE_OK) {
            fprintf(stderr, "Erro na requisição: %s\n", curl_easy_strerror(res));
        } else {
            printf("Requisição enviada com sucesso!\n");
        }

        // Libera os recursos
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Erro na inicialização do libcurl\n");
    }

    return 0;
}

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

        int status = 0;
        if (Correto(tabulIn, tam)){
            status = 1;
            printf("**RESULTADO CORRETO**\n");
        }
        else{
            printf("**RESULTADO ERRADO**\n");
        }

        save_elastic_data(pow, t2 - t1, status);

        t3 = wall_time();
        printf("tam=%d; tempos: init=%7.7f, comp=%7.7f, fim=%7.7f, tot=%7.7f \n",
               tam, t1 - t0, t2 - t1, t3 - t2, t3 - t0);
        free(tabulIn);
        free(tabulOut);
    }

}

void parse_message(const char* buffer, int* num1, int* num2) {
    // Find the position of '<' and '>'
    const char* start = strchr(buffer, '<');
    const char* end = strchr(buffer, '>');
    
    if (start == NULL || end == NULL) {
        fprintf(stderr, "Invalid message format: %s\n", buffer);
        *num1 = *num2 = -1; // Set numbers to invalid values
        return;
    }
    
    // Extract the numbers from the buffer
    int len = end - start - 1;
    char* numbers = (char*)malloc(len + 1);
    strncpy(numbers, start + 1, len);
    numbers[len] = '\0';
    
    // Convert the numbers to integers
    sscanf(numbers, "%d,%d", num1, num2);
    
    // Free allocated memory
    free(numbers);
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[1024] = {0};
    int valread;

    // Read and respond with "Hello" to the client
    const char *hello = "Hello from server!";
    write(client_socket, hello, strlen(hello));

    // Communicate with the client until it disconnects
    while ((valread = read(client_socket, buffer, 1024)) > 0) {
        // Print the received message from the client
        printf("Received message: %s\n", buffer);

        int num1, num2;
    
        parse_message(buffer, &num1, &num2);

        if (num1 >=3 && num2 <= 10)
            solveProblem(num1, num2);

        // Respond with "Hello" again
        write(client_socket, hello, strlen(hello));
        
        memset(buffer, 0, sizeof(buffer));
    }

    if (valread == 0) {
        printf("Client disconnected\n");
    } else {
        perror("read");
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the given IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("New client connected\n");

        // Create a new thread to handle communication with the client
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void *)&new_socket) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
