# Use uma imagem base leve do Linux
FROM alpine:latest

# Instale o compilador C, a biblioteca de desenvolvimento do curl e o utilitário curl no container
RUN apk update && apk add gcc libc-dev curl-dev curl

# Copie o arquivo do código C para o container
COPY jogoVidaOMP.c /app/

# Compile o código C e gere o binário do servidor
RUN gcc -o /app/jogoVidaOMP /app/jogoVidaOMP.c -lpthread -fopenmp -lcurl

# Defina o diretório de trabalho
WORKDIR /app

EXPOSE 1234

# Comando para executar o servidor socket TCP/IP quando o container for iniciado
CMD ["./jogoVidaOMP"]
