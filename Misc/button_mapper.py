from __future__ import print_function
from inputs import get_gamepad
import time

keys_dict = {}

def main():
    cnt = 0
    while cnt < 10000:
        #print("Hi")
        events = get_gamepad()
        for event in events:
            #if (event.ev_type == 'Key'):
            #if (event.ev_type != 'Sync' and event.ev_type != 'Absolute'):
            #if ('ABS_HAT' in event.code):
            #if (event.ev_type != 'Sync'):
            #    if (event.code == 'ABS_RY'):
            #        print(event.ev_type, event.code, event.state)

            pad_event = ""
            if (event.ev_type == 'Key'):
                if (event.code == 'BTN_NORTH'):
                    pad_event += "Triangle: "
                elif (event.code == 'BTN_WEST'):
                    pad_event += "Square: "
                elif (event.code == 'BTN_SOUTH'):
                    pad_event += "Cross: "
                elif (event.code == 'BTN_EAST'):
                    pad_event += "Circle: "
                elif (event.code == 'BTN_TL'):
                    pad_event += "Left Bumper: "
                elif (event.code == 'BTN_TR'):
                    pad_event += "Right Bumper: "
                elif (event.code == 'BTN_START'):
                    pad_event += "Share: "
                elif (event.code == 'BTN_SELECT'):
                    pad_event += "Options: "
                elif (event.code == 'BTN_THUMBL'):
                    pad_event += "Left Stick: "
                elif (event.code == 'BTN_THUMBR'):
                    pad_event += "Right Stick: "

                if (event.state == 1):
                    pad_event += "Pressed"
                elif (event.state == 0):
                    pad_event += "Released"

            if (event.ev_type == 'Absolute' and "ABS_HAT0" in event.code):
                if (event.code == 'ABS_HAT0X' and event.state == 0):
                    pad_event = "Dpad-X: Released"
                elif (event.code == 'ABS_HAT0X' and event.state < 0):
                    pad_event = "Dpad Left: Pressed"
                elif (event.code == 'ABS_HAT0X' and event.state > 0):
                    pad_event = "Dpad Right: Pressed"
                elif (event.code == 'ABS_HAT0Y' and event.state == 0):
                    pad_event = "Dpad-Y: Released"
                elif (event.code == 'ABS_HAT0Y' and event.state < 0):
                    pad_event = "Dpad Up: Pressed"
                elif (event.code == 'ABS_HAT0Y' and event.state > 0):
                    pad_event = "Dpad Down: Pressed"
            
            if (pad_event != ""):
                print(pad_event)
        #print(cnt)
        cnt += 1

if __name__ == "__main__":
    main()