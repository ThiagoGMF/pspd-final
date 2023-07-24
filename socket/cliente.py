import asyncio

async def receive_message(reader):
    while True:
        data = await reader.read(1024)
        if not data:
            break
        message = data.decode().strip()
        print(f"Mensagem recebida do servidor: {message}")

async def send_message(writer):
    while True:
        message = input("Digite a mensagem (ou 'sair' para sair): ")
        if message.lower() == "sair":
            break
        writer.write(message.encode())
        await writer.drain()

    writer.close()
    await writer.wait_closed()

async def main():
    reader, writer = await asyncio.open_connection('127.0.0.1', 52028)

    asyncio.create_task(receive_message(reader))
    await send_message(writer)

if __name__ == "__main__":
    asyncio.run(main())
