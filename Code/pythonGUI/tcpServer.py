import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = ('192.168.1.206', 10000)
print(f"starting up on port {server_address}")
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)

while True:
    # Wait for a connection
    print("waiting for a connection")
    connection, client_address = sock.accept()
    try:
        print(f"connection from {client_address}")

        # Receive the data in small chunks and retransmit it
        while True:
            message = input(">>> ")
            connection.sendall(message.encode('utf-8'))
            print(f"sent {message}")

            data = connection.recv(64)
            print(f"received {data}")
    
    finally:
        # Clean up the connection
        connection.close()