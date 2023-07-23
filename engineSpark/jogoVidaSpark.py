from pyspark.sql import SparkSession
import asyncio
import requests
import uuid

url = "http://elasticsearch-service:9200/openmp/_doc/"


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

  for i in range(2*(tam-3)):
    UmaVida(tabulIn, tabulOut, tam)
    UmaVida(tabulOut, tabulIn, tam)
  if (Correto(tabulIn, tam)):
    print("*Ok, RESULTADO CORRETO*\n", potencia)
  else:
    print("**Not Ok, RESULTADO ERRADO**\n", potencia)
	guid = str(uuid.uuid4())
	data = {"mode": "Spark", "potency": potencia, time: 1, status: "correct" if Correto(tabulIn, tam) else "incorrect"}
	enviarRequisicaPut(url + guid, data)
POWMIN = 3
POWMAX = 6

a = list(range(POWMIN,POWMAX+1))

spark = SparkSession.builder.master("local[6]").appName("MyProgram").getOrCreate()
a = spark.sparkContext.parallelize(a)

a.foreach(lambda x: jogoVida(x))

def enviarRequisicaPut(url, data):
    try:
        response = requests.put(url, json=data)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Erro na requisição PUT: {e}")
        return None
