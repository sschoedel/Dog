from smbus import SMBus
import socket
import time

 
addr = 0x8 # bus address
bus = SMBus(1) # indicates /dev/i2c-1
 
numb = 1

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ('192.168.4.20', 10000)
print(f"connecting to port {server_address}")

sock.connect(server_address)

try:
    while True:
        data = sock.recv(64)
        print(f"received {data}")
        
        response = ""

        expected_data = 40

        if len(data) != expected_data:
            print(f"Input is {len(data)} ints long. Must be {expected_data} ({int(expected_data/4)} sets of 4)")		
        else:
            vals = []
            for i in range(int(expected_data/4)):
                # Take sets of 4 integers and remove '0x' from hex() value
                hexState = hex(int(data[i*4:(i+1)*4]))[2:]

                # Forward pad sets of 4 hex values with 0s
                while len(hexState) < 4:
                    hexState = "0" + hexState
                
                print(f"state hex: {hexState}")

                vals.append(int(hexState[0:2],16))
                vals.append(int(hexState[2:4],16))

            print(f"vals: {vals}")
            bus.write_block_data(addr, 0x2, vals)
            time.sleep(1.0/1000.0)
            # response = "message passed to Teensy over I2C"

        # sock.sendall(response.encode('utf-8'))

finally:
    print("closing socket")
    sock.close()
    
