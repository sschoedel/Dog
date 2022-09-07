# Released by rdb under the Unlicense (unlicense.org)
# Further reading about the WinMM Joystick API:
# http://msdn.microsoft.com/en-us/library/windows/desktop/dd757116(v=vs.85).aspx

from math import floor, ceil
import ctypes
import winreg
from ctypes.wintypes import WORD, UINT, DWORD
from ctypes.wintypes import WCHAR as TCHAR

class XBOXCONTROLLER():
    # Fetch function pointers
    joyGetNumDevs = ctypes.windll.winmm.joyGetNumDevs
    joyGetPos = ctypes.windll.winmm.joyGetPos
    joyGetPosEx = ctypes.windll.winmm.joyGetPosEx
    joyGetDevCaps = ctypes.windll.winmm.joyGetDevCapsW


    JOY_RETURNX = 0x1
    JOY_RETURNY = 0x2
    JOY_RETURNZ = 0x4
    JOY_RETURNR = 0x8
    JOY_RETURNU = 0x10
    JOY_RETURNV = 0x20
    JOY_RETURNPOV = 0x40
    JOY_RETURNBUTTONS = 0x80
    JOY_RETURNRAWDATA = 0x100
    JOY_RETURNPOVCTS = 0x200
    JOY_RETURNCENTERED = 0x400
    JOY_USEDEADZONE = 0x800
    JOY_RETURNALL = JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ | JOY_RETURNR | JOY_RETURNU | JOY_RETURNV | JOY_RETURNPOV | JOY_RETURNBUTTONS

    # This is the mapping for my XBox 360 controller.
    button_names = ['a', 'b', 'x', 'y', 'tl', 'tr', 'back', 'start', 'thumbl', 'thumbr']
    povbtn_names = ['dpad_up', 'dpad_right', 'dpad_down', 'dpad_left']

    # Define some structures from WinMM that we will use in function calls.
    class JOYCAPS(ctypes.Structure):
        # Define constants
        MAXPNAMELEN = 32
        MAX_JOYSTICKOEMVXDNAME = 260
        _fields_ = [
            ('wMid', WORD),
            ('wPid', WORD),
            ('szPname', TCHAR * MAXPNAMELEN),
            ('wXmin', UINT),
            ('wXmax', UINT),
            ('wYmin', UINT),
            ('wYmax', UINT),
            ('wZmin', UINT),
            ('wZmax', UINT),
            ('wNumButtons', UINT),
            ('wPeriodMin', UINT),
            ('wPeriodMax', UINT),
            ('wRmin', UINT),
            ('wRmax', UINT),
            ('wUmin', UINT),
            ('wUmax', UINT),
            ('wVmin', UINT),
            ('wVmax', UINT),
            ('wCaps', UINT),
            ('wMaxAxes', UINT),
            ('wNumAxes', UINT),
            ('wMaxButtons', UINT),
            ('szRegKey', TCHAR * MAXPNAMELEN),
            ('szOEMVxD', TCHAR * MAX_JOYSTICKOEMVXDNAME),
        ]

    class JOYINFO(ctypes.Structure):
        _fields_ = [
            ('wXpos', UINT),
            ('wYpos', UINT),
            ('wZpos', UINT),
            ('wButtons', UINT),
        ]

    class JOYINFOEX(ctypes.Structure):
        _fields_ = [
            ('dwSize', DWORD),
            ('dwFlags', DWORD),
            ('dwXpos', DWORD),
            ('dwYpos', DWORD),
            ('dwZpos', DWORD),
            ('dwRpos', DWORD),
            ('dwUpos', DWORD),
            ('dwVpos', DWORD),
            ('dwButtons', DWORD),
            ('dwButtonNumber', DWORD),
            ('dwPOV', DWORD),
            ('dwReserved1', DWORD),
            ('dwReserved2', DWORD),
        ]

    def __init__(self):
        # Get the number of supported devices (usually 16).
        num_devs = self.joyGetNumDevs()
        if num_devs == 0:
            print("Joystick driver not loaded.")

        # Number of the joystick to open.
        joy_id = 0

        # Check if the joystick is plugged in.
        self.info = self.JOYINFO()
        self.p_info = ctypes.pointer(self.info)
        if self.joyGetPos(0, self.p_info) != 0:
            print("Joystick %d not plugged in." % (joy_id + 1))

        # Get device capabilities.
        self.caps = self.JOYCAPS()
        if self.joyGetDevCaps(joy_id, ctypes.pointer(self.caps), ctypes.sizeof(self.JOYCAPS)) != 0:
            print("Failed to get device capabilities.")

        print("Driver name:", self.caps.szPname)

        # Fetch the name from registry.
        key = None
        if len(self.caps.szRegKey) > 0:
            try:
                key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, "System\\CurrentControlSet\\Control\\MediaResources\\Joystick\\%s\\CurrentJoystickSettings" % (self.caps.szRegKey))
            except WindowsError:
                key = None

        if key:
            oem_name = winreg.QueryValueEx(key, "Joystick%dOEMName" % (joy_id + 1))
            if oem_name:
                key2 = winreg.OpenKey(winreg.HKEY_CURRENT_USER, "System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\%s" % (oem_name[0]))
                if key2:
                    oem_name = winreg.QueryValueEx(key2, "OEMName")
                    print("OEM name:", oem_name[0])
                key2.Close()

        # Set the initial button states.
        self.button_states = {}
        for b in range(self.caps.wNumButtons):
            name = self.button_names[b]
            if (1 << b) & self.info.wButtons:
                self.button_states[name] = True
            else:
                self.button_states[name] = False

        for name in self.povbtn_names:
            self.button_states[name] = False

        self.buttons_text = ""

        # Initialise the JOYINFOEX structure.
        self.info = self.JOYINFOEX()
        self.info.dwSize = ctypes.sizeof(self.JOYINFOEX)
        self.info.dwFlags = self.JOY_RETURNBUTTONS | self.JOY_RETURNCENTERED | self.JOY_RETURNPOV | self.JOY_RETURNU | self.JOY_RETURNV | self.JOY_RETURNX | self.JOY_RETURNY | self.JOY_RETURNZ
        self.p_info = ctypes.pointer(self.info)
