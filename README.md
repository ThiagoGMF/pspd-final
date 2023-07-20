# pspd-final

## Rodar os deployments no k8s

- kubectl apply -f k8s/deployments/<nome-do-arquivo>

## Rodar os services no k8s

- kubectl apply -f k8s/server/<nome-do-arquivo>
- kubectl expose deployment <nome-do-deployment> --type=LoadBalancer --name <nome-do-servico> --port=<porta>

## Acessar os logs

- kubectl logs -f deployment.apps/tcp-server-deployment

## Ver os pods

- kubectl get pods

## Ver os services

- kubectl get service

## Executar os servicos

- minikube service <nome-do-servico>
- minikube service --url <nome-do-servico>

## Buildar imagem docker e subir no Docker Hub

- docker build -t <username-docker>/<nome-da-imagem>:<versao> .
- docker run -p <porta-local>:<porta-do-container> <nome-da-imagem>::<versao>
- docker push <username-docker>/<nome-da-imagem>:<versao>

## Usar imagem do Docker Hub no k8s

- docker.io/<username-docker>/<nome-da-imagem>:<versao>
