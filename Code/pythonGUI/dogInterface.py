import socket
import sys
from appJar import gui

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = ('192.168.1.206', 10000)
print(f"starting up on port {server_address}")
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)

print("waiting for a connection")
connection, client_address = sock.accept()
print(f"accepted connection from {client_address}")

app = gui()


def updateCmd(title):
    cmd = ""
    values = app.getAllScales()
    # print(f"{title} value: {values[title]}")
    for i, val in enumerate(values):
        inp = str(values[val])
        while len(inp) < 4:
            inp = "0" + inp
        cmd += inp
        # print(f"i, val, values[{val}]: {i}, {val}, {values[val]}")
    # print(f"cmd: {cmd}")

    connection.sendall(cmd.encode('utf-8'))
    print(f"sent {cmd}")

    data = connection.recv(64)
    print(f"received {data}")

    
app.addLabel("title", "remote dog interface")
app.setLabelBg("title", "lightgray")

app.addLabelScale("roll")
app.setScaleRange("roll", 0, 1023, curr=512)

app.addLabelScale("pitch")
app.setScaleRange("pitch", 0, 1023, curr=512)

app.addLabelScale("yaw")
app.setScaleRange("yaw", 0, 1023, curr=512)

app.addLabelScale("forward")
app.setScaleRange("forward", 0, 1023, curr=512)

app.addLabelScale("lateral")
app.setScaleRange("lateral", 0, 1023, curr=512)

app.addLabelScale("vertical")
app.setScaleRange("vertical", 0, 1023, curr=512)

app.addLabelScale("X vel")
app.setScaleRange("X vel", 0, 1023, curr=512)

app.addLabelScale("Y vel")
app.setScaleRange("Y vel", 0, 1023, curr=512)

app.addLabelScale("Rotation vel")
app.setScaleRange("Rotation vel", 0, 1023, curr=512)

app.setScaleChangeFunction("vertical", updateCmd)
app.setScaleChangeFunction("lateral", updateCmd)
app.setScaleChangeFunction("forward", updateCmd)
app.setScaleChangeFunction("roll", updateCmd)
app.setScaleChangeFunction("pitch", updateCmd)
app.setScaleChangeFunction("yaw", updateCmd)
app.setScaleChangeFunction("Y vel", updateCmd)
app.setScaleChangeFunction("X vel", updateCmd)
app.setScaleChangeFunction("Rotation vel", updateCmd)

app.go()