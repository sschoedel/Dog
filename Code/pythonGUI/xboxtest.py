import xbox
import time

xb = xbox.XBOXCONTROLLER()

# Fetch new joystick data until it returns non-0 (that is, it has been unplugged)
while xb.joyGetPosEx(0, xb.p_info) == 0:
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

    # Display the x, y, trigger values.
    print("\r(% .3f % .3f % .3f) (% .3f % .3f % .3f)%s%s" % (x, y, lt, rx, ry, rt, xb.buttons_text, erase), end='')

    #print info.dwXpos, info.dwYpos, info.dwZpos, info.dwRpos, info.dwUpos, info.dwVpos, info.dwButtons, info.dwButtonNumber, info.dwPOV, info.dwReserved1, info.dwReserved2
    time.sleep(0.01)