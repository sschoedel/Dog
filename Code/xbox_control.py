from smbus import SMBus
from xbox_driver_rpi import XboxController
import time

addr = 0x8 # bus address
bus = SMBus(1) # indicates /dev/i2c-1

xb = XboxController()

numb = 1

while True:
    data = "0512051205120512051205120512051205120000"
    data_v = list(data)
    # Get xbox data
    left_x_raw, left_y_raw, right_x_raw, right_y_raw = xb.read()
    if xb.joyGetPosEx(0, xb.p_info) != 0:
        print("Xbox not connected")
    else:
        x_vel = ( (left_x_raw - 32767) / 32768.0 ) * 1024
        y_vel = ( (left_y_raw - 32767) / 32768.0 ) * 1024
        rot_vel = ( (right_x_raw - 32767) / 32768.0 ) * 1024
        _ = ( (right_y_raw - 32767) / 32768.0 ) * 1024
        data[24:28] = ''.join(str(x_vel))
        data[28:32] = ''.join(str(y_vel))
        data[32:36] = ''.join(str(rot_vel))
        data[36:40] = 1 # for walking mode

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
