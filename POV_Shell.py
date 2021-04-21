import sys, cmd, struct
import asyncio
from bleak import BleakClient
from bleak import exc
import threading, queue
import time
import logging
import multiprocessing
from inputs import get_gamepad
from datetime import datetime

#Message Commands
GET_DISPLAY_SIZE =        0
GET_BUFFER_TYPE =         1
SET_BUFFER_TYPE =         2
CLEAR_DISPLAY =           3
UPDATE_DISPLAY =          4
GET_PERIOD =              5
BUTTON_EVENT =            6
GET_REGISTER =            7
SET_REGISTER =            8
SET_MARQUEE_TEXT =        9
USE_MARQEE =              10
GET_RTC_TIME =            11
SET_RTC_TIME =            12
GET_EXEC_STATE =          13
SET_EXEC_STATE =          14
JOYSTICK_EVENT =          15
TRIGGER_EVENT =           16

#Message Responses
ACK =                    0
NACK =                   1
GET_DISPLAY_SIZE_RESP =  2
GET_BUFFER_TYPE_RESP =   3
GET_PERIOD_RESP =        4
LOG_MSG =                5
GET_REGISTER_RESP =      6
GET_RTC_TIME_RESP =      7
GET_EXEC_STATE_RESP =    8

MSG_CMD_NUM = 15
RESP_CMD_NUM = 9

#Button events
BTN_PRESS = 0
BTN_RELEASE = 1
BTN_TAP = 2

#Button Keys
BTN_KEY_TRIANGLE = 0
BTN_KEY_SQUARE = 1
BTN_KEY_CROSS = 2
BTN_KEY_CIRCLE = 3
BTN_KEY_LBUMPER = 4
BTN_KEY_RBUMPER = 5
BTN_KEY_LSTICK = 6
BTN_KEY_RSTICK = 7
BTN_KEY_SHARE = 8
BTN_KEY_OPTIONS = 9
BTN_KEY_DUP = 10
BTN_KEY_DLEFT = 11
BTN_KEY_DDOWN = 12
BTN_KEY_DRIGHT = 13
BTN_KEY_DX = 14
BTN_KEY_DY = 15

#ABS Pad Events
L_STICK =   0
R_STICK =   1
L_TRIG =    2
R_TRIG =    3


address = 'F0:C7:7F:94:CC:6C'
NAME_UUID = "00002a00-0000-1000-8000-00805f9b34fb"
WRITE_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"

TIMEOUT_PERIOD = 5.0

messages_to_display = []
messages_from_display = []
client = None
bt_process = None

parent_conn = None 
child_conn_g = None

msg_footer = b'\xBE\xEF'

msg_buf = bytearray()
mutex = threading.Lock()
LOGGING_ENABLED = False
LOG_EN = 'LOG_ENABLED'
LOG_DIS = 'LOG_DISABLED'

msg_queue_dict = {}


def getMessage(message_id, payload):
    if (payload is None):
        payload_size = 0
        header = struct.pack('i i', payload_size, message_id)
        return (header + msg_footer)
    else:
        payload_size = len(payload)
        header = struct.pack('i i', payload_size, message_id)
        return (header + payload + msg_footer)

def getMessageHeader(message):
    return struct.unpack('i i', message[0:8]) 

def getMessagePayload(message):
    return message[8:-2]

def retrieveMessage(conn):
    if (conn.poll(TIMEOUT_PERIOD)):
        resp_data = conn.recv()
        if (len(resp_data) < 8):
            return False
        (payload_size, msg_id) = getMessageHeader(resp_data)
        while (len(resp_data) < (payload_size + 10)):#header size = 8, footer = 2
            if (conn.poll(TIMEOUT_PERIOD)):
                temp = conn.recv()
                resp_data += temp
            else:
                return False
        return [resp_data, (payload_size, msg_id)]
    else:
        return False

def notification_handler(sender, data):
    global child_conn_g
    global msg_buf
    global mutex
    global LOGGING_ENABLED
    #print("Message received:")
    #print("{0}: {1}".format(sender, data))
    if (child_conn_g is None):
        print("Error: invalid pipe connection")
    else:
        mutex.acquire()
        msg_buf += data
        while (len(msg_buf) >= 8):
            (payload_size, msg_id) = getMessageHeader(msg_buf)
            msg_size = payload_size + 10
            if (len(msg_buf) >= msg_size):
                #Complete message
                message = msg_buf[:msg_size]
                #print("Full Message:", message)
                if (msg_id == LOG_MSG):
                    if (LOGGING_ENABLED == True):
                        print(getMessagePayload(message).decode('utf-8'))
                else:
                    child_conn_g.send(message)
                msg_buf = msg_buf[msg_size:]
            else:
                break
        mutex.release()

def message_helper(send_msg_id, send_payload, resp_msg_id):
    out_msg = getMessage(send_msg_id, send_payload)
    parent_conn.send(out_msg)

    resp = msg_queue_dict[resp_msg_id].get(timeout=TIMEOUT_PERIOD)
    (payload_size, msg_id) = getMessageHeader(resp)

    print("Received message: %d of size %d bytes"%(msg_id, payload_size))

    if (msg_id == NACK):
        print("Error: command NACKed")
        msg_queue_dict[resp_msg_id].task_done()
        return None
    elif (msg_id != resp_msg_id):
        print("Error: Unexpected command response")
        msg_queue_dict[resp_msg_id].task_done()
        return None

    payload = getMessagePayload(resp)
    print(payload)
    return payload


def button_event(event, button_idx, resp):
    if (not str(event).isdigit()):
        return -1
    if (not str(button_idx).isdigit()):
        return -1
    if (resp == True):
        resp = 1
    else:
        resp = 0

    payload = struct.pack('i i i', event, button_idx, resp)

    #Setup message that doesn't expect response
    if (resp == 0):
        out_msg = getMessage(BUTTON_EVENT, payload)
        parent_conn.send(out_msg)
        return 0

    #Setup message that expect response
    response_id = ACK
    try:
        payload = message_helper(BUTTON_EVENT, payload, response_id)
        if (payload is None):
            return -1
        print("Successfully sent button event")
        msg_queue_dict[response_id].task_done()
        return 0
    except queue.Empty as e:
            print("Error: Timed out waiting for response")
            return -1

def joystick_event(joystick_type, x, y, resp):
    if (not str(joystick_type).isdigit()):
        return -1
    if (not str(x).isdigit()):
        return -1
    if (not str(y).isdigit()):
        return -1
    if (resp == True):
        resp = 1
    else:
        resp = 0

    payload = struct.pack('i i i i', joystick_type, x, y, resp)

    #Setup message that doesn't expect response
    if (resp == 0):
        out_msg = getMessage(JOYSTICK_EVENT, payload)
        parent_conn.send(out_msg)
        return 0

    #Setup message that expect response
    response_id = ACK
    try:
        payload = message_helper(JOYSTICK_EVENT, payload, response_id)
        if (payload is None):
            return -1
        print("Successfully sent joystick event")
        msg_queue_dict[response_id].task_done()
        return 0
    except queue.Empty as e:
            print("Error: Timed out waiting for response")
            return -1

def trigger_event(trigger_type, trigger, resp):
    if (not str(trigger_type).isdigit()):
        return -1
    if (not str(trigger).isdigit()):
        return -1
    if (resp == True):
        resp = 1
    else:
        resp = 0

    payload = struct.pack('i i i', trigger_type, trigger, resp)

    #Setup message that doesn't expect response
    if (resp == 0):
        out_msg = getMessage(TRIGGER_EVENT, payload)
        parent_conn.send(out_msg)
        return 0

    #Setup message that expect response
    response_id = ACK
    try:
        payload = message_helper(TRIGGER_EVENT, payload, response_id)
        if (payload is None):
            return -1
        print("Successfully sent trigger event")
        msg_queue_dict[response_id].task_done()
        return 0
    except queue.Empty as e:
            print("Error: Timed out waiting for response")
            return -1

class POV_Shell(cmd.Cmd):
    #intro = 'POV Shell: Have fun'
    intro = r"""
---------------------------------------------------------------------------------------------------------------------
      ___           ___           ___                    ___           ___           ___           ___       ___ 
     /\  \         /\  \         /\__\                  /\  \         /\__\         /\  \         /\__\     /\__\
    /::\  \       /::\  \       /:/  /                 /::\  \       /:/  /        /::\  \       /:/  /    /:/  /
   /:/\:\  \     /:/\:\  \     /:/  /                 /:/\ \  \     /:/__/        /:/\:\  \     /:/  /    /:/  / 
  /::\~\:\  \   /:/  \:\  \   /:/__/  ___            _\:\~\ \  \   /::\  \ ___   /::\~\:\  \   /:/  /    /:/  /  
 /:/\:\ \:\__\ /:/__/ \:\__\  |:|  | /\__\          /\ \:\ \ \__\ /:/\:\  /\__\ /:/\:\ \:\__\ /:/__/    /:/__/   
 \/__\:\/:/  / \:\  \ /:/  /  |:|  |/:/  /          \:\ \:\ \/__/ \/__\:\/:/  / \:\~\:\ \/__/ \:\  \    \:\  \   
      \::/  /   \:\  /:/  /   |:|__/:/  /            \:\ \:\__\        \::/  /   \:\ \:\__\    \:\  \    \:\  \  
       \/__/     \:\/:/  /     \::::/__/              \:\/:/  /        /:/  /     \:\ \/__/     \:\  \    \:\  \ 
                  \::/  /       ~~~~                   \::/  /        /:/  /       \:\__\        \:\__\    \:\__\
                   \/__/                                \/__/         \/__/         \/__/         \/__/     \/__/
---------------------------------------------------------------------------------------------------------------------
Type help or ? to list commands
---------------------------------------------------------------------------------------------------------------------
            """
    prompt = '>>'
    file = None

    def do_test(self, arg):
        'Test Command'
        print("Test 1")
    
    def do_get_display_size(self, arg):
        'Returns the dimensions of the display'
        try:
            response_id = GET_DISPLAY_SIZE_RESP
            payload = message_helper(GET_DISPLAY_SIZE, None, response_id)
            if (payload is None):
                return 
            (l, w, h) = struct.unpack('i i i', payload) 
            print("Length: %d, Width: %d, Height: %d"%(l, w, h))
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_get_buffer_type(self, arg):
        'Returns whether single or double buffering is used'
        try:
            response_id = GET_BUFFER_TYPE_RESP
            payload = message_helper(GET_BUFFER_TYPE, None, response_id)
            if (payload is None):
                return

            buffer_type = struct.unpack('c', payload)
            buffer_type = int(buffer_type[0][0]) 
            print("buffer type:", buffer_type)
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_set_buffer_type(self, arg):
        'Sets whether single or double buffering is used: set_buffer_type <single | double>'
        try:
            #Parse Args
            arg = arg.split()
            if (arg[0] in ['1', 's', 'S', 'single', 'Single', 'SINGLE']):
                type = 1
            elif (arg[0] in ['0', 'd', 'D', 'double', 'Double', 'DOUBLE']):
                type = 0
            else:
                raise Exception("Invalid buffer type: {}".format(arg[0]))
            payload = struct.pack('c', bytes([type]))

            #Setup message
            response_id = ACK
            payload = message_helper(SET_BUFFER_TYPE, payload, response_id)
            if (payload is None):
                return
            print("Buffer type set successfully")
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_clear_display(self, arg):
        'Clears the display buffer (and update): clear_display <update (true/false)>'
        try:
            #Parse Args
            arg = arg.split()
            if (arg[0] in ['1', 't', 'T', 'true', 'True', 'TRUE']):
                update = 1
            elif (arg[0] in ['0', 'f', 'F', 'false', 'False', 'FALSE']):
                update = 0
            else:
                raise Exception("Invalid boolean: {}".format(arg[0]))
            payload = struct.pack('c', bytes([update]))

            #Setup message
            response_id = ACK
            payload = message_helper(CLEAR_DISPLAY, payload, response_id)
            if (payload is None):
                return
            print("Display successfully cleared")
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())
    
    def do_update_display(self, arg):
        'Update display (switch front and rear buffers)'
        try:
            #Setup message
            response_id = ACK
            payload = message_helper(UPDATE_DISPLAY, None, response_id)
            if (payload is None):
                return
            print("Display successfully updated")
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())
    
    def do_get_period(self, arg):
        'Returns last recorded rotation period'
        try:
            response_id = GET_PERIOD_RESP
            payload = message_helper(GET_PERIOD, None, response_id)
            if (payload is None):
                return
            (period) = struct.unpack('i', payload) 
            print("Period: %u us"%(period))      
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_button_event(self, arg):
        'Sends "button" command to display: button_event <button idx> <press | release | tap>'
        try:
            #Parse Args
            arg = arg.split()
            if (not str(arg[0]).isdigit()):
                raise Exception("Invalid button index: {}".format(arg[0]))
            button_idx = int(arg[0])

            if (arg[1] in ['0', 'p', 'P', 'press', 'Press', 'PRESS']):
                event = 0
            elif (arg[1] in ['1', 'r', 'R', 'release', 'Release', 'RELEASE']):
                event = 1
            elif (arg[1] in ['2', 't', 'T', 'tap', 'Tap', 'TAP']):
                event = 2
            else:
                raise Exception("Invalid button event: {}".format(arg[1]))

            if (button_event(event, button_idx, True) != 0):
                print("Error sending button event")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_joystick_event(self, arg):
        'Sends "joystick" command to display: joystick_event <left | right> <x> <y>'
        try:
            #Parse Args
            arg = arg.split()
            if (arg[0].upper() in ['L', 'LEFT']):
                joystick_type = L_STICK
            elif (arg[0].upper() in ['R', 'RIGHT']):
                joystick_type = R_STICK
            else:
                raise Exception("Invalid joystick location: {}".format(arg[0]))

            if (not str(arg[1]).isdigit()):
                raise Exception("Invalid joystick value: {}".format(arg[1]))
            joystick_x = int(arg[1])

            if (not str(arg[2]).isdigit()):
                raise Exception("Invalid joystick value: {}".format(arg[2]))
            joystick_y = int(arg[2])

            if (joystick_event(joystick_type, x, y, True) != 0):
                print("Error sending button event")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_trigger_event(self, arg):
        'Sends "trigger" command to display: trigger_event <left | right> <trig_val>'
        try:
            #Parse Args
            arg = arg.split()
            if (arg[0].upper() in ['L', 'LEFT']):
                trigger_type = L_TRIG
            elif (arg[0].upper() in ['R', 'RIGHT']):
                trigger_type = R_TRIG
            else:
                raise Exception("Invalid trigger location: {}".format(arg[0]))

            if (not str(arg[1]).isdigit()):
                raise Exception("Invalid trigger index: {}".format(arg[1]))
            trig_val = int(arg[1])

            if (trigger_event(trigger_type, trig_val, True) != 0):
                print("Error sending button event")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_get_register(self, arg):
        'Returns specified register value: get_register <reg_addr>'
        try:
            #Parse Args
            arg = arg.split()
            addr = int(arg[0], 16)
            payload = struct.pack('i', addr)

            response_id = GET_REGISTER_RESP
            payload = message_helper(GET_REGISTER, payload, response_id)
            if (payload is None):
                return
            (reg_val,) = struct.unpack('i', payload) 
            print("Reg[%08X]: %08X"%(addr, reg_val))      
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())
    
    def do_set_register(self, arg):
        'Sets specified register value: set_register <reg_addr> <reg_val>'
        try:
            #Parse Args
            arg = arg.split()
            addr = int(arg[0], 16)
            reg_val = int(arg[1], 16)
            payload = struct.pack('i i', addr, reg_val)

            response_id = ACK
            payload = message_helper(SET_REGISTER, payload, response_id)
            if (payload is None):
                return
            print("Reg[%08X] updated to %08X successfully"%(addr, reg_val))      
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    #do_set_marqee_text

    #use_marquee

    def do_get_rtc_time(self, arg):
        'Returns RTC time on display'
        try:
            response_id = GET_RTC_TIME_RESP
            payload = message_helper(GET_RTC_TIME, None, response_id)
            if (payload is None):
                return
            (h, m, s) = struct.unpack('i i i', payload) 
            print("RTC time = (%02d:%02d:%02d)"%(h, m, s))      
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_set_rtc_time(self, arg):
        'Set RTC time on display to current time'
        try:
            time = str(datetime.now().time())
            time = time.split(":")
            hours = int(time[0])
            mins = int(time[1])
            secs = int(float(time[2]))

            payload = struct.pack('i i i', hours, mins, secs)

            response_id = ACK
            payload = message_helper(SET_RTC_TIME, payload, response_id)
            if (payload is None):
                return
            print("Successfully set RTC time = (%02d:%02d:%02d)"%(hours, mins, secs))      
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_get_exec_state(self, arg):
        'Returns current execution state of the display'
        try:
            state_map = ["POV_SCRATCH_LOOP", "POV_TEST", "DS4_TEST", "SPACE_GAME"]
            response_id = GET_EXEC_STATE_RESP
            payload = message_helper(GET_EXEC_STATE, None, response_id)
            if (payload is None):
                return
            (state,) = struct.unpack('i', payload) 
            print("Exec state = %s(%d)"%(state_map[state], state))      
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_set_exec_state(self, arg):
        'Sets current execution state of the display: set_execution_state <state>'
        try:
            #Parse Args
            arg = arg.split()
            if (not str(arg[0]).isdigit()):
                raise Exception("Invalid button index: {}".format(arg[0]))
            state = int(arg[0])
            payload = struct.pack('i', state)

            response_id = ACK
            payload = message_helper(SET_EXEC_STATE, payload, response_id)
            if (payload is None):
                return
            print("Successfully set execution state: %d"%(state))      
            msg_queue_dict[response_id].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_enable_logging(self, arg):
        'Enables printing of LOG messages from display'
        try:
            parent_conn.send(LOG_EN)
        except BaseException:
            print("Error:", sys.exc_info())
    
    def do_disable_logging(self, arg):
        'Disables printing of LOG messages from display'
        try:
            parent_conn.send(LOG_DIS)
        except BaseException:
            print("Error:", sys.exc_info())

    def do_test2(self, arg):
        'Test2 Command'
        print("Test 2")
    
    def do_exit(self, arg):
        'Exit shell'
        global bt_process
        if (bt_process is not None):
            bt_process.terminate()
        self.close()
        return True

    #--------------------------
    def do_record(self, arg):
        'Save future commands to filename:  RECORD rose.cmd'
        self.file = open(arg, 'w')
    def do_playback(self, arg):
        'Playback commands from a file:  PLAYBACK rose.cmd'
        self.close()
        with open(arg) as f:
            self.cmdqueue.extend(f.read().splitlines())
    def precmd(self, line):
        line = line.lower()
        if self.file and 'playback' not in line:
            print(line, file=self.file)
        return line
    def close(self):
        if self.file:
            self.file.close()
            self.file = None

    def do_connect(self, arg):
        'Connect to display'
        pass

    def do_disconnect(self, arg):
        'Disconnect from display'
        pass

def parse(arg):
    'Convert a series of zero or more numbers to an argument tuple'
    return tuple(map(int, arg.split()))


async def run_notification_and_name(address, loop, child_conn):
    print("Establishing connection...")
    global LOGGING_ENABLED
    connect_attempts = 10
    while (connect_attempts > 0):
        try:
            async with BleakClient(address) as client:
                x = await client.is_connected()
                print("Connected: {0}".format(x))

                model_name = await client.read_gatt_char(NAME_UUID)
                print("Model Number: {0}".format("".join(map(chr, model_name))))
                await client.start_notify(WRITE_UUID, notification_handler)

                while(True):
                    if (child_conn.poll()):
                        data = child_conn.recv()
                        if (data == LOG_EN):
                            print("Enabling LOGGING...")
                            LOGGING_ENABLED = True
                        elif (data == LOG_DIS):
                            print("Disabling LOGGING...")
                            LOGGING_ENABLED = False
                        else:
                            data = bytearray(data)
                            data_len = len(data)
                            print("Data Size:", data_len)

                            while(1):
                                if (len(data) <= 20):
                                    await client.write_gatt_char(WRITE_UUID, data)
                                    break
                                else:
                                    send_data = data[:20]
                                    data = data[20:]
                                    await client.write_gatt_char(WRITE_UUID, send_data)
                    await asyncio.sleep(0.001)

                await client.stop_notify(WRITE_UUID)
        except exc.BleakError:
            print("Retrying connection...")
            connect_attempts -= 1
            await asyncio.sleep(0.100)
        except BaseException:
            print("Error:", sys.exc_info())
    if (connect_attempts == 0):
        print("Failed to connect...")


def ble_process(name, child_conn):
    global child_conn_g
    child_conn_g = child_conn
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.create_task(run_notification_and_name(address, loop, child_conn))
    loop.run_forever()

def message_router():
    while True:
        message = retrieveMessage(parent_conn)
        if (message != False):
            resp = message[0]
            payload_size, msg_id = message[1]
            msg_queue_dict[msg_id].put(resp)
        else:
            time.sleep(0.1)

def gamepad_handler():
    print("In gamepad handler")
    connect_attempts = 3
    prev_lx = None
    prev_ly = None
    prev_rx = None
    prev_ry = None

    prev_ltrig = None
    prev_rtrig = None

    STICK_THRESH = 5000
    TRIG_THRESH = 5

    while 1:
        try:
            while 1:
                events = get_gamepad()
                for event in events:
                    pad_event = ""
                    btn_event = -1
                    btn_key = -1

                    new_lstick = False
                    new_rstick = False
                    new_ltrig = False
                    new_rtrig = False

                    if (event.ev_type == 'Key'):
                        if (event.code == 'BTN_NORTH'):
                            pad_event += "Triangle: "
                            btn_key = BTN_KEY_TRIANGLE
                        elif (event.code == 'BTN_WEST'):
                            pad_event += "Square: "
                            btn_key = BTN_KEY_SQUARE
                        elif (event.code == 'BTN_SOUTH'):
                            pad_event += "Cross: "
                            btn_key = BTN_KEY_CROSS
                        elif (event.code == 'BTN_EAST'):
                            pad_event += "Circle: "
                            btn_key = BTN_KEY_CIRCLE
                        elif (event.code == 'BTN_TL'):
                            pad_event += "Left Bumper: "
                            btn_key = BTN_KEY_LBUMPER
                        elif (event.code == 'BTN_TR'):
                            pad_event += "Right Bumper: "
                            btn_key = BTN_KEY_RBUMPER
                        elif (event.code == 'BTN_START'):
                            pad_event += "Share: "
                            btn_key = BTN_KEY_SHARE
                        elif (event.code == 'BTN_SELECT'):
                            pad_event += "Options: "
                            btn_key = BTN_KEY_OPTIONS
                        elif (event.code == 'BTN_THUMBL'):
                            pad_event += "Left Stick: "
                            btn_key = BTN_KEY_LSTICK
                        elif (event.code == 'BTN_THUMBR'):
                            pad_event += "Right Stick: "
                            btn_key = BTN_KEY_RSTICK

                        if (event.state == 1):
                            pad_event += "Pressed"
                            btn_event = BTN_PRESS
                        elif (event.state == 0):
                            pad_event += "Released"
                            btn_event = BTN_RELEASE

                    elif (event.ev_type == 'Absolute' and "ABS_HAT0" in event.code):
                        if (event.code == 'ABS_HAT0X' and event.state == 0):
                            pad_event = "Dpad-X: Released"
                            btn_key = BTN_KEY_DX
                            btn_event = BTN_RELEASE
                        elif (event.code == 'ABS_HAT0X' and event.state < 0):
                            pad_event = "Dpad Left: Pressed"
                            btn_key = BTN_KEY_DLEFT
                            btn_event = BTN_PRESS
                        elif (event.code == 'ABS_HAT0X' and event.state > 0):
                            pad_event = "Dpad Right: Pressed"
                            btn_key = BTN_KEY_DRIGHT
                            btn_event = BTN_PRESS
                        elif (event.code == 'ABS_HAT0Y' and event.state == 0):
                            pad_event = "Dpad-Y: Released"
                            btn_key = BTN_KEY_DY
                            btn_event = BTN_RELEASE
                        elif (event.code == 'ABS_HAT0Y' and event.state < 0):
                            pad_event = "Dpad Up: Pressed"
                            btn_key = BTN_KEY_DUP
                            btn_event = BTN_PRESS
                        elif (event.code == 'ABS_HAT0Y' and event.state > 0):
                            pad_event = "Dpad Down: Pressed"
                            btn_key = BTN_KEY_DDOWN
                            btn_event = BTN_PRESS

                    elif (event.ev_type == 'Absolute'):
                        if (event.code == 'ABS_X'):
                            if (prev_lx == None or (abs(prev_lx - event.state) > STICK_THRESH)):
                                prev_lx = event.state
                                new_lstick = True
                        elif (event.code == 'ABS_Y'):
                            if (prev_ly == None or (abs(prev_ly - event.state) > STICK_THRESH)):
                                prev_ly = event.state
                                new_lstick = True
                        elif (event.code == 'ABS_RX'):
                            if (prev_rx == None or (abs(prev_rx - event.state) > STICK_THRESH)):
                                prev_rx = event.state
                                new_rstick = True
                        elif (event.code == 'ABS_RY'):
                            if (prev_ry == None or (abs(prev_ry - event.state) > STICK_THRESH)):
                                prev_ry = event.state
                                new_rstick = True
                        elif (event.code == 'ABS_Z'):
                            if (prev_ltrig == None or (abs(prev_ltrig - event.state) > TRIG_THRESH)):
                                prev_ltrig = event.state
                                new_ltrig = True
                        elif (event.code == 'ABS_RZ'):
                            if (prev_rtrig == None or (abs(prev_rtrig - event.state) > TRIG_THRESH)):
                                prev_rtrig = event.state
                                new_rtrig = True

                    if (new_lstick == True and prev_lx != None and prev_ly != None):
                        joystick_event(L_STICK, prev_lx, prev_ly, False)
                        pad_event = "Left Stick: (%d,%d)"%(prev_lx, prev_ly)
                    if (new_rstick == True and prev_rx != None and prev_ry != None):
                        joystick_event(R_STICK, prev_rx, prev_ry, False)
                        pad_event = "Right Stick: (%d,%d)"%(prev_rx, prev_ry)

                    if (new_ltrig == True and prev_ltrig != None):
                        trigger_event(L_TRIG, prev_ltrig, False)
                        pad_event = "Left Trigger: %d"%(prev_ltrig)
                    if (new_rtrig == True and prev_rtrig != None):
                        trigger_event(R_TRIG, prev_rtrig, False)
                        pad_event = "Right Trigger: %d"%(prev_rtrig)


                    if (pad_event != ""):
                        print(pad_event)
                    if (btn_event != -1 and btn_key != -1):
                        button_event(btn_event, btn_key, False)
        except:
            print("Error: Gamepad not found...")
            connect_attempts -= 1
            if (connect_attempts <= 0):
                print("Giving up connecting to gamepad")
                return 
            time.sleep(20)

if __name__ == '__main__':
    parent_conn, child_conn = multiprocessing.Pipe()
    bt_process = multiprocessing.Process(target=ble_process, args=(1, child_conn))
    bt_process.start()

    for i in range(RESP_CMD_NUM):
        msg_queue_dict[i] = queue.Queue()

    threading.Thread(target=message_router, daemon=True).start()
    threading.Thread(target=gamepad_handler, daemon=True).start()

    POV_Shell().cmdloop()



    
