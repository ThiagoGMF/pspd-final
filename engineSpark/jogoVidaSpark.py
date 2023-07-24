from pyspark.sql import SparkSession
import asyncio
import requests
import uuid
import re
import time
from datetime import datetime

url = "http://elasticsearch-service:9200/jogovida/_create/"
connected_clients = []
a = 0

   
def enviarRequisicao(url, data):
    try:
        response = requests.post("http://elasticsearch-service:9200/jogovida/_doc", json=data)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"Erro na requisição: {e}")
    return None

def extrairNumeros(texto):
    padrao = r'<(\d+),(\d+)>'
    correspondencias = re.search(padrao, texto)
    if correspondencias:
        numero1 = int(correspondencias.group(1))
        numero2 = int(correspondencias.group(2))
        if( numero1 > numero2):
            temp = numero1
            numero1 = numero2
            numero2 = temp
        return (numero1, numero2)
    else:
        return None, None

def matrix(m, n):
    return [[0 for x in range(row * n, (row + 1) * n)] for row in range(m)]

def Correto(tabul, tam):
  cnt = 0
  for i in range(tam+2):
    for j in range(tam+2):
      cnt += tabul[i][j]
  return cnt == 5 and tabul[tam-2][tam-1] and tabul[tam-1][tam] and tabul[tam][tam-2] and tabul[tam][tam-1] and tabul[tam][tam]

def InitTabul(tam):
  tabulIn = matrix(tam, tam)
  tabulOut = matrix(tam, tam)
  tabulIn[1][2] = 1
  tabulIn[2][3] = 1
  tabulIn[3][1] = 1
  tabulIn[3][2] = 1
  tabulIn[3][3] = 1
  return (tabulIn, tabulOut)

def DumpTabul(tabul, tam, first, last, msg):
    for i in range(first, last+1):
        print("=", end="")
        print("‎")
    for i in range(first,last+1):
        for j in range(first, last+1):
            print('X' if tabul[i][j] == 1 else '.', end='')
        print('|')
    for i in range(first, last+1):
        print("=", end="")

def UmaVida(tabulIn, tabulOut,tam):
  for i in range(1,tam+1):
    for j in range(1, tam+1):
      vizviv = tabulIn[i-1][j-1] + tabulIn[i-1][j] + tabulIn[i-1][j+1] + tabulIn[i][j-1] + tabulIn[i][j+1] + tabulIn[i+1][j-1] + tabulIn[i+1][j] + tabulIn[i+1][j+1];
      if (tabulIn[i][j] and vizviv < 2):
        tabulOut[i][j] = 0
      elif (tabulIn[i][j] and vizviv > 3):
        tabulOut[i][j] = 0
      elif(not tabulIn[i][j] and vizviv == 3):
        tabulOut[i][j] = 1
      else:
        tabulOut[i][j] = tabulIn[i][j]

def jogoVida(potencia):
  tam = 1 << potencia
  tabulIn, tabulOut = InitTabul(tam+2)
  t1 = datetime.now()
  for i in range(2*(tam-3)):
    UmaVida(tabulIn, tabulOut, tam)
    UmaVida(tabulOut, tabulIn, tam)
  t2 = datetime.now()
  delta_tempo = t2 - t1
  if (Correto(tabulIn, tam)):
    print("*Ok, RESULTADO CORRETO*\n", potencia)
  else:
    print("**Not Ok, RESULTADO ERRADO**\n", potencia)
  guid = str(uuid.uuid4())
  print("link: ", guid)
  json = {"status": 1 if Correto(tabulIn, tam) else 0,"mode": "Spark","time": delta_tempo.total_seconds(),"potency": potencia}
  print("ENVIANDO=", json)
  enviarRequisicao(url, json)

async def handle_client(reader, writer):
    connected_clients.append(writer)
    addr = writer.get_extra_info('peername')
    print(f"Novo cliente conectado: {addr}")

    try:
        data = await reader.read(1024)

        message = data.decode().strip()
        print(f"Mensagem recebida do cliente {addr}: {message}")

        # Enviar a mensagem para todos os clientes conectados
        for client in connected_clients:
            if client != writer and not client.is_closing():
                client.write(data)
                await client.drain()

        # Enviar a mensagem para o servidor remoto
        menor, maior = extrairNumeros(message)
        if(menor != None):
            menor = int(menor)
            maior = int (maior)
            print(menor, maior, "-------------")
            if(menor> 2 or maior < 11):   
                a = list(range(menor,maior+1))
                a = spark.sparkContext.parallelize(a)
                a.foreach(lambda x: jogoVida(x))
    except asyncio.CancelledError:
        pass
    except ConnectionError:
        pass
    finally:
        print(f"Cliente {addr} desconectado.")
        try:
            writer.close()
            await writer.wait_closed()
        except asyncio.CancelledError:
            pass
        connected_clients.remove(writer)

async def main():
    server = await asyncio.start_server(handle_client, '0.0.0.0', 7071)

    addr = server.sockets[0].getsockname()
    print(f"Servidor iniciado em {addr}")

    async with server:
        await server.serve_forever()

if __name__ == "__main__":
    global spark
    spark = SparkSession.builder.master("local[6]").appName("MyProgram").getOrCreate()
    asyncio.run(main())

spark = ""


