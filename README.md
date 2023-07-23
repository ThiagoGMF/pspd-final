# pspd-final

## Executar os pods

- kubectl apply -f k8s/app.yaml

## Apagar os deployments e services

- kubectl delete deployments --all
- kubectl delete services --all

## Acessar os logs

- kubectl logs -f deployment.apps/tcp-server-deployment

## Ver os pods

- kubectl get pods

## Ver os services

- kubectl get service

## Ver tudo

- kubectl get all

## Executar os servicos

- minikube service <nome-do-servico>
- minikube service --url <nome-do-servico>

## Expor as portas
kubectl expose deployment elasticsearch --type=LoadBalancer --name elasticsearch --port=9200

kubectl expose deployment kibana --type=LoadBalancer --name kibana --port=5601

## Buildar imagem docker e subir no Docker Hub

- docker build -t <username-docker>/<nome-da-imagem>:<versao> .
- docker run -p <porta-local>:<porta-do-container> <nome-da-imagem>::<versao>
- docker push <username-docker>/<nome-da-imagem>:<versao>

## Usar imagem do Docker Hub no k8s

- docker.io/<username-docker>/<nome-da-imagem>:<versao>
