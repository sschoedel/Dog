import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ('192.168.1.206', 10000)
print(f"connecting to port {server_address}")
sock.connect(server_address)
try:
    # Send data
    message = "012301230123012301230123012301230123"
    print(f"sending {message}")
    sock.sendall(message.encode('utf-8'))

    # Look for the response
    amount_received = 0
    amount_expected = len(message)
    
    while amount_received < amount_expected:
        data = sock.recv(64)
        amount_received += len(data)
        print(f"received {data}")

finally:
    print("closing socket")
    sock.close()