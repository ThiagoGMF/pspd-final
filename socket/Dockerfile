# Use uma imagem Python oficial como base
FROM python:3.9

# Copie o código do servidor para o diretório de trabalho no contêiner
COPY server.py /app/server.py

# Instale as dependências, se houver
# Exemplo: se você precisa de outras bibliotecas Python, adicione um arquivo requirements.txt e use:
# COPY requirements.txt /app/requirements.txt
# RUN pip install --no-cache-dir -r /app/requirements.txt

# Defina o diretório de trabalho
WORKDIR /app
RUN pip install requests

EXPOSE 8888

# Comando para executar o servidor Python quando o contêiner for iniciado
CMD ["python3","-u","server.py"]
