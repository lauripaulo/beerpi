from asyncio.windows_events import NULL
import socket

HOST = "192.168.1.118"  # The server's hostname or IP address
PORT = 333  # The port used by the server
            
def simple_exchange():
    message = NULL
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as conn:
        conn.connect((HOST, PORT))
        while (True):
            message = input("Enter message: ")
            conn.sendall(b"{message}")
            if (message == "exit!"):
                break
            data = conn.recv(1024)
            print(f"Message received: {data}")
        


if __name__ == "__main__":
    simple_exchange()