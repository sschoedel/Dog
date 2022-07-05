import socket
import sys
import time
from appJar import gui
import xbox
import time

# xb = xbox.XBOXCONTROLLER()

# Set nominal values for commands
noms = {"rollNom" : 512, "pitchNom" : 512, "yawNom" : 512, "forNom":512, "latNom":512, "vertNom":512, "xVelNom":512, "yVelNom":512, "rotVelNom":512}
modes = {"orient" : 0, "walk" : 1}

class Tracker():
    x_curr = 0
    y_curr = 0
    x_init = 0
    y_init = 0
    x_disp = 0
    y_disp = 0
    tracking = False
    mode = 0

tracker = Tracker()

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = ('192.168.4.20', 10000)
print(f"starting up on port {server_address}")
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)

print("waiting for a connection")
connection, client_address = sock.accept()
print(f"accepted connection from {client_address}")

def updateCmd(title):
    cmd = ""
    values = app.getAllScales()
    values["mode"] = tracker.mode
    # print(f"values: {values}")
    # print(f"{title} value: {values[title]}")
    for i, val in enumerate(values):
        
        inp = str(values[val])
        while len(inp) < 4:
            inp = "0" + inp
        cmd += inp
        # print(f"i, val, values[{val}]: {i}, {val}, {values[val]}")
    # print(f"cmd: {cmd}")

    connection.sendall(cmd.encode('utf-8'))
    # print(f"sent {cmd}")

    # data = connection.recv(64)
    # print(f"received {data}")

def resetOrients():
    app.setScale("roll", noms["rollNom"])
    app.setScale("pitch", noms["pitchNom"])
    app.setScale("yaw", noms["yawNom"])
    app.setScale("forward", noms["forNom"])
    app.setScale("lateral", noms["latNom"])
    app.setScale("vertical", noms["vertNom"])
    app.setScale("X vel", noms["xVelNom"])

def resetVels():
    app.setScale("Y vel", noms["yVelNom"])
    app.setScale("Rotation vel", noms["rotVelNom"])

def createCircle(x,y,r,outline="gray",width=1):
    canvas.create_oval(x-r, y-r, x+r, y+r, outline=outline, width=width)


def motion(event):
    tracker.x_curr = event.x
    tracker.y_curr = event.y
    # print(f"x, y: {tracker.x_disp, tracker.y_disp}")
    if tracker.tracking:
        # Compute displacement
        tracker.x_disp = tracker.x_curr - tracker.x_init
        tracker.y_disp = tracker.y_curr - tracker.y_init
        # Draw displacement interface
        canvas.delete("all")
        canvas.create_oval(tracker.x_curr-5, tracker.y_curr-5, tracker.x_curr+5, tracker.y_curr+5, outline="gray", width=1)
        canvas.create_oval(tracker.x_init-5, tracker.y_init-5, tracker.x_init+5, tracker.y_init+5, fill="gray", outline="gray", width=1)
        canvas.create_line(tracker.x_init, tracker.y_init, tracker.x_curr, tracker.y_curr, dash=123)
    else:
        tracker.x_disp = 0
        tracker.y_disp = 0

def startClick(event):
    tracker.tracking = True
    tracker.x_init = tracker.x_curr
    tracker.y_init = tracker.y_curr
    tracker.x_disp = tracker.x_curr - tracker.x_init
    tracker.y_disp = tracker.y_curr - tracker.y_init
    print(f"start x, y: {tracker.x_disp, tracker.y_disp}")
    canvas.create_oval(tracker.x_curr-5, tracker.y_curr-5, tracker.x_curr+5, tracker.y_curr+5, outline="gray", width=1)
        
def stopClick(event):
    tracker.tracking = False
    resetVels()
    tracker.x_disp = 0
    tracker.y_disp = 0
    canvas.delete("all")
    # print(f"canvas width: {canvas.w}")
    

def setCmd():
    # app.setScale("roll", 512+tracker.x_disp/400*1023)
    # app.setScale("pitch", 512+tracker.x_disp/400*1023)
    # app.setScale("yaw", 512+tracker.x_disp/400*1023)
    # app.setScale("forward", 512+tracker.x_disp/400*1023)
    # app.setScale("lateral", 512+tracker.x_disp/400*1023)
    # app.setScale("vertical", 512+tracker.x_disp/400*1023)
    # app.setScale("X vel", 512+tracker.y_disp/400*1023)
    

    """
    xb.joyGetPosEx(0, xb.p_info)
    # Remap the values to float
    x = (xb.info.dwXpos - 32767) / 32768.0
    y = (xb.info.dwYpos - 32767) / 32768.0
    trig = (xb.info.dwZpos - 32767) / 32768.0
    rx = (xb.info.dwRpos - 32767) / 32768.0
    ry = (xb.info.dwUpos - 32767) / 32768.0

    # NB.  Windows drivers give one axis for the trigger, but I want to have
    # two for compatibility with platforms that do support them as separate axes.
    # This means it'll behave strangely when both triggers are pressed, though.
    lt = max(-1.0,  trig * 2 - 1.0)
    rt = max(-1.0, -trig * 2 - 1.0)

    # Figure out which buttons are pressed.
    for b in range(xb.caps.wNumButtons):
        pressed = (0 != (1 << b) & xb.info.dwButtons)
        name = xb.button_names[b]
        xb.button_states[name] = pressed

    # Determine the state of the POV buttons using the provided POV angle.
    if xb.info.dwPOV == 65535:
        povangle1 = None
        povangle2 = None
    else:
        angle = xb.info.dwPOV / 9000.0
        povangle1 = int(xb.floor(angle)) % 4
        povangle2 = int(xb.ceil(angle)) % 4

    for i, btn in enumerate(xb.povbtn_names):
        if i == povangle1 or i == povangle2:
            xb.button_states[btn] = True
        else:
            xb.button_states[btn] = False

    # Format a list of currently pressed buttons.
    prev_len = len(xb.buttons_text)
    xb.buttons_text = " "
    for btn in xb.button_names + xb.povbtn_names:
        if xb.button_states.get(btn):
            xb.buttons_text += btn + ' '

    # Add spaces to erase data from the previous line
    erase = ' ' * max(0, prev_len - len(xb.buttons_text))
"""
    # Display the x, y, trigger values.
    # print("\r(% .3f % .3f % .3f) (% .3f % .3f % .3f)%s%s" % (x, y, lt, rx, ry, rt, xb.buttons_text, erase), end='')

    #print info.dwXpos, info.dwYpos, info.dwZpos, info.dwRpos, info.dwUpos, info.dwVpos, info.dwButtons, info.dwButtonNumber, info.dwPOV, info.dwReserved1, info.dwReserved2
    # time.sleep(0.01)

    # yval = 1023-(y+1)/2*1023
    # xval = (x+1)/2*1023

    # dzone = 400
    # if yval < dzone and yval > -dzone: yval = 0
    # if xval < dzone and xval > -dzone: xval = 0

    # app.setScale("Y vel", yval)
    # app.setScale("Rotation vel", xval)

    app.setScale("Y vel", 512-tracker.y_disp/400*1023)
    app.setScale("Rotation vel", 512+tracker.x_disp/400*1023)
    updateCmd("none")

def changeMode(title):
    tracker.mode = modes[app.getRadioButton(title)]
    print(f"tracker.mode: {tracker.mode}")
    updateCmd("none")


app = gui()
canvas = app.addCanvas("c1")
canvas.bind("<Motion>", motion, add="+")
canvas.bind("<ButtonPress-1>", startClick, add="+")
canvas.bind("<ButtonRelease-1>", stopClick, add="+")

app.registerEvent(setCmd)
app.setPollTime(100) # Update commands at 1000/20 = 50 Hz

app.addLabel("title", "remote dog interface")
app.setLabelBg("title", "lightgray")

app.addLabelScale("roll")
app.setScaleRange("roll", 0, 1023, curr=noms["rollNom"])

app.addLabelScale("pitch")
app.setScaleRange("pitch", 0, 1023, curr=noms["pitchNom"])

app.addLabelScale("yaw")
app.setScaleRange("yaw", 0, 1023, curr=noms["yawNom"])

app.addLabelScale("forward")
app.setScaleRange("forward", 0, 1023, curr=noms["forNom"])

app.addLabelScale("lateral")
app.setScaleRange("lateral", 0, 1023, curr=noms["latNom"])

app.addLabelScale("vertical")
app.setScaleRange("vertical", 0, 1023, curr=noms["vertNom"])

app.addLabelScale("X vel")
app.setScaleRange("X vel", 0, 1023, curr=noms["xVelNom"])

app.addLabelScale("Y vel")
app.setScaleRange("Y vel", 0, 1023, curr=noms["yVelNom"])

app.addLabelScale("Rotation vel")
app.setScaleRange("Rotation vel", 0, 1023, curr=noms["rotVelNom"])

app.addRadioButton("mode", "orient")
app.addRadioButton("mode", "walk")

app.addButton("reset orientation", resetOrients)


# Set callbacks
app.setScaleChangeFunction("vertical", updateCmd)
app.setScaleChangeFunction("lateral", updateCmd)
app.setScaleChangeFunction("forward", updateCmd)
app.setScaleChangeFunction("roll", updateCmd)
app.setScaleChangeFunction("pitch", updateCmd)
app.setScaleChangeFunction("yaw", updateCmd)
app.setScaleChangeFunction("Y vel", updateCmd)
app.setScaleChangeFunction("X vel", updateCmd)
app.setScaleChangeFunction("Rotation vel", updateCmd)

app.setRadioButtonChangeFunction("mode", changeMode)

app.go()