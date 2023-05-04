from smbus import SMBus
from xbox_driver_rpi import XboxController
import time

addr = 0x8 # bus address
bus = SMBus(1) # indicates /dev/i2c-1

xb = XboxController()

numb = 1

max_js_value = 32767.0

while True:
    data = "0512051205120512051205120512051205120000"
    data_v = list(data)

    # Get xbox data
    left_x_raw, left_y_raw, right_x_raw, right_y_raw = xb.read()
    x_vel = int( left_x_raw / max_js_value * 1024)
    y_vel = int( left_y_raw / max_js_value * 1024)
    rot_vel = int( right_x_raw / max_js_value * 1024)
    _ = int( right_y_raw / max_js_value * 1024)

    # Prepare data for sending (convert int to string, pad with zeros until length=4)
    x_vel_str = str(x_vel)
    while len(x_vel_str) < 4:
        x_vel_str = "0" + x_vel_str
    y_vel_str = str(y_vel)
    while len(y_vel_str) < 4:
        y_vel_str = "0" + y_vel_str
    rot_vel_str = str(rot_vel)
    while len(rot_vel_str) < 4:
        rot_vel_str = "0" + rot_vel_str
    
    # Convert data string to list of characters
    data_l = list(data)
    data_l[24:28] = x_vel_str
    data_l[28:32] = y_vel_str
    data_l[32:36] = rot_vel_str
    data_l[39] = str(1) # for walking mode

    # Convert data list back to one long string
    data = ''.join(data_l)

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
