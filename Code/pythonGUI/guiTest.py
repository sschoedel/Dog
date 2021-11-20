from appJar import gui

class Tracker():
    x_curr = 0
    y_curr = 0
    x_init = 0
    y_init = 0
    x_disp = 0
    y_disp = 0
    tracking = False

tracker = Tracker()

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
    print(f"cmd: {cmd}")

def resetCmds():
    app.setScale("roll", 512)
    app.setScale("pitch", 512)
    app.setScale("yaw", 512)
    app.setScale("forward", 512)
    app.setScale("lateral", 512)
    app.setScale("vertical", 512)
    app.setScale("X vel", 512)
    app.setScale("Y vel", 512)
    app.setScale("Rotation vel", 512)

def createCircle(x,y,r,outline="gray",width=1):
    canvas.create_oval(x-r, y-r, x+r, y+r, outline=outline, width=width)


def motion(event):
    tracker.x_curr = event.x
    tracker.y_curr = event.y
    tracker.x_disp = tracker.x_curr - tracker.x_init
    tracker.y_disp = tracker.y_curr - tracker.y_init
    print(f"x, y: {tracker.x_disp, tracker.y_disp}")
    if tracker.tracking:
        # app.setScale("roll", 512+tracker.x_disp/400*1023)
        # app.setScale("pitch", 512+tracker.x_disp/400*1023)
        # app.setScale("yaw", 512+tracker.x_disp/400*1023)
        # app.setScale("forward", 512+tracker.x_disp/400*1023)
        # app.setScale("lateral", 512+tracker.x_disp/400*1023)
        # app.setScale("vertical", 512+tracker.x_disp/400*1023)
        app.setScale("X vel", 512-tracker.y_disp/400*1023)
        # app.setScale("Y vel", 512+tracker.x_disp/400*1023)
        app.setScale("Rotation vel", 512+tracker.x_disp/400*1023)
        canvas.delete("all")
        canvas.create_oval(tracker.x_curr-5, tracker.y_curr-5, tracker.x_curr+5, tracker.y_curr+5, outline="gray", width=1)
        canvas.create_oval(tracker.x_init-5, tracker.y_init-5, tracker.x_init+5, tracker.y_init+5, fill="gray", outline="gray", width=1)
        canvas.create_line(tracker.x_init, tracker.y_init, tracker.x_curr, tracker.y_curr, dash=123)

def startClick(event):
    tracker.tracking = True
    tracker.x_init = tracker.x_curr
    tracker.y_init = tracker.y_curr
    tracker.x_disp = tracker.x_curr - tracker.x_init
    tracker.y_disp = tracker.y_curr - tracker.y_init
    print(f"x, y: {tracker.x_disp, tracker.y_disp}")
    canvas.create_oval(tracker.x_curr-5, tracker.y_curr-5, tracker.x_curr+5, tracker.y_curr+5, outline="gray", width=1)
        
def stopClick(event):
    tracker.tracking = False
    print(f"hi")
    resetCmds()
    canvas.delete("all")

app = gui()
canvas = app.addCanvas("c1")
# canvas.create_oval(10, 10, 100, 100, fill="red", outline="blue", width=3)
# canvas.create_line(0, 255, 255, 0, dash=123)
# app.setCanvasOverFunction("c1", [enter, leave])
canvas.bind("<Motion>", motion, add="+")
canvas.bind("<ButtonPress-1>", startClick, add="+")
canvas.bind("<ButtonRelease-1>", stopClick, add="+")

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