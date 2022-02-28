from smbus import SMBus
import time

 
addr = 0x8 # bus address
bus = SMBus(1) # indicates /dev/i2c-1
 
numb = 1

while True:
    # your code here
    # read current center location of object from camera
    # compute difference between center of camera and center of object
    # figure out roll pitch and yaw to achieve alignment

    pitch_num = 512
    roll_num = 512
    
    long_str = str(pitch_num) + str(roll_num)
    

    data = ["0512051205120512051205120512051205120512"]
    
    expected_data = 40

    if len(data) != expected_data:
        # response = "Input is " + str(len(data)) + " ints long. Must be 36 (9 sets of 4)"
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
