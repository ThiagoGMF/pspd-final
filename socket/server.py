import asyncio

connected_clients = []
remote_server_host = 'openmp-service'
remote_server_port = 1234

remote_server_host_spark = 'spark-app-service'
remote_server_port_spark = 7071

async def handle_client(reader, writer):
    connected_clients.append(writer)
    addr = writer.get_extra_info('peername')
    print(f"Novo cliente conectado: {addr}")

    try:
        while True:
            data = await reader.read(1024)
            if not data:
                break

            message = data.decode().strip()
            print(f"Mensagem recebida do cliente {addr}: {message}")

            # Enviar a mensagem para todos os clientes conectados
            for client in connected_clients:
                if client != writer and not client.is_closing():
                    client.write(data)
                    await client.drain()

            # Enviar a mensagem para o servidor remoto
            await send_to_remote_server(message)
            await send_to_remote_server2(message)
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

async def send_to_remote_server(message):
    reader, writer = await asyncio.open_connection(remote_server_host, remote_server_port)
    writer.write(message.encode())
    await writer.drain()
    writer.close()
    await writer.wait_closed()

async def send_to_remote_server2(message):
    reader, writer = await asyncio.open_connection(remote_server_host_spark, remote_server_port_spark)
    writer.write(message.encode())
    await writer.drain()
    writer.close()
    await writer.wait_closed()

async def main():
    server = await asyncio.start_server(handle_client, '0.0.0.0', 8888)
    
    addr = server.sockets[0].getsockname()
    print(f"Servidor iniciado em {addr}")

    async with server:
        await server.serve_forever()

if __name__ == "__main__":
    asyncio.run(main())
