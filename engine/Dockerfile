# Use uma imagem base leve do Linux
FROM alpine:latest

# Copie o arquivo do código C para o container
COPY jogoVidaOMP.c /app/

# Instale o compilador C no container
RUN apk update && apk add gcc libc-dev

# Compile o código C e gere o binário do servidor
RUN gcc -o /app/jogoVidaOMP /app/jogoVidaOMP.c -lpthread -fopenmp

# Defina o diretório de trabalho
WORKDIR /app

EXPOSE 1234

# Comando para executar o servidor socket TCP/IP quando o container for iniciado
CMD ["./jogoVidaOMP"]
