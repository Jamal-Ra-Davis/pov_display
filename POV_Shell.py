import sys, cmd, struct
import asyncio
from bleak import BleakClient
from bleak import exc
import threading, queue
import time
import logging
import multiprocessing

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

#Message Responses
ACK =                    0
NACK =                   1
GET_DISPLAY_SIZE_RESP =  2
GET_BUFFER_TYPE_RESP =   3
GET_PERIOD_RESP =        4
LOG_MSG =                5
GET_REGISTER_RESP =      6

MSG_CMD_NUM = 11
RESP_CMD_NUM = 7

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
            out_msg = getMessage(GET_DISPLAY_SIZE, None)
            parent_conn.send(out_msg)
            #message = retrieveMessage(parent_conn)
            resp = msg_queue_dict[GET_DISPLAY_SIZE_RESP].get(timeout=TIMEOUT_PERIOD)
            (payload_size, msg_id) = getMessageHeader(resp)

            #if (message is False):
            #    print("Error: Failed to get message")
            #    return
            #resp = message[0]
            #payload_size, msg_id = message[1]
            print("Received message: %d of size %d bytes"%(msg_id, payload_size))

            if (msg_id == NACK):
                print("Error: command NACKed")
                msg_queue_dict[GET_DISPLAY_SIZE_RESP].task_done()
                return
            elif (msg_id != GET_DISPLAY_SIZE_RESP):
                print("Error: Unexpected command response")
                msg_queue_dict[GET_DISPLAY_SIZE_RESP].task_done()
                return

            payload = getMessagePayload(resp)
            print(payload)
            handled = True
            (l, w, h) = struct.unpack('i i i', payload) 
            print("Length: %d, Width: %d, Height: %d"%(l, w, h))
            msg_queue_dict[GET_DISPLAY_SIZE_RESP].task_done()
        except queue.Empty as e:
            print("Error: Timed out waiting for response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_get_buffer_type(self, arg):
        'Returns whether single or double buffering is used'
        try:
            out_msg = getMessage(GET_BUFFER_TYPE, None)
            parent_conn.send(out_msg)
            message = retrieveMessage(parent_conn)
            if (message is False):
                print("Error: Failed to get message")
                return
            resp = message[0]
            payload_size, msg_id = message[1]
            print("Received message: %d of size %d bytes"%(msg_id, payload_size))

            if (msg_id == NACK):
                print("Error: command NACKed")
                return
            elif (msg_id != GET_BUFFER_TYPE_RESP):
                print("Error: Unexpected command response")
                return

            payload = getMessagePayload(resp)
            print(payload)
            handled = True
            buffer_type = struct.unpack('c', payload)
            buffer_type = int(buffer_type[0][0]) 
            print("buffer type:", buffer_type)
        except BaseException:
            print("Error:", sys.exc_info())

    def do_set_buffer_type(self, arg):
        'Sets whether single or double buffering is used: set_buffer_type <single | double>'
        try:
            arg = arg.split()
            if (arg[0] in ['1', 's', 'S', 'single', 'Single', 'SINGLE']):
                type = 1
            elif (arg[0] in ['0', 'd', 'D', 'double', 'Double', 'DOUBLE']):
                type = 0
            else:
                raise Exception("Invalid buffer type: {}".format(arg[0]))
            payload = struct.pack('c', bytes([type]))
            out_msg = getMessage(SET_BUFFER_TYPE, payload)
            print(out_msg)

            parent_conn.send(out_msg)
            message = retrieveMessage(parent_conn)
            if (message is False):
                print("Error: Failed to get message")
                return
            resp = message[0]
            payload_size, msg_id = message[1]
            if (msg_id == ACK):
                print("Buffer type set successfully")
            elif (msg_id == NACK):
                print("Error: command NACKed")
            else:
                print("Error: Unexpected command response")
        except BaseException:
            print("Error:", sys.exc_info())

    def do_clear_display(self, arg):
        'Clears the display buffer (and update): clear_display <update (true/false)>'
        try:
            arg = arg.split()
            if (arg[0] in ['1', 't', 'T', 'true', 'True', 'TRUE']):
                update = 1
            elif (arg[0] in ['0', 'f', 'F', 'false', 'False', 'FALSE']):
                update = 0
            else:
                raise Exception("Invalid boolean: {}".format(arg[0]))

            payload = struct.pack('c', bytes([update]))
            out_msg = getMessage(CLEAR_DISPLAY, payload)
            print(out_msg)

            parent_conn.send(out_msg)
            message = retrieveMessage(parent_conn)
            if (message is False):
                print("Error: Failed to get message")
                return
            resp = message[0]
            payload_size, msg_id = message[1]
            if (msg_id == ACK):
                print("Display successfully cleared")
            elif (msg_id == NACK):
                print("Error: command NACKed")
            else:
                print("Error: Unexpected command response")
        except BaseException:
            print("Error:", sys.exc_info())
    
    def do_update_display(self, arg):
        'Update display (switch front and rear buffers)'
        try:
            out_msg = getMessage(UPDATE_DISPLAY, None)
            print(out_msg)

            parent_conn.send(out_msg)
            message = retrieveMessage(parent_conn)
            if (message is False):
                print("Error: Failed to get message")
                return
            resp = message[0]
            payload_size, msg_id = message[1]
            if (msg_id == ACK):
                print("Display successfully updated")
            elif (msg_id == NACK):
                print("Error: command NACKed")
            else:
                print("Error: Unexpected command response")
            pass
        except BaseException:
            print("Error:", sys.exc_info())
    
    def do_get_period(self, arg):
        'Returns last recorded rotation period'
        try:
            out_msg = getMessage(GET_PERIOD, None)
            print(out_msg)

            parent_conn.send(out_msg)
            message = retrieveMessage(parent_conn)
            if (message is False):
                print("Error: Failed to get message")
                return
            resp = message[0]
            payload_size, msg_id = message[1]
            print("Received message: %d of size %d bytes"%(msg_id, payload_size))

            if (msg_id == NACK):
                print("Error: command NACKed")
                return
            elif (msg_id != GET_PERIOD_RESP):
                print("Error: Unexpected command response")
                return

            payload = getMessagePayload(resp)
            print(payload)
            handled = True
            (period) = struct.unpack('i', payload) 
            print("Period: %u us"%(period))
        except BaseException:
            print("Error:", sys.exc_info())

    def do_button_event(self, arg):
        'Sends "button" command to display: button_event <button idx> <press | release | tap>'
        try:
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
            
            payload = struct.pack('i i', event, button_idx)
            out_msg = getMessage(BUTTON_EVENT, payload)
            print(out_msg)

            parent_conn.send(out_msg)
            message = retrieveMessage(parent_conn)
            if (message is False):
                print("Error: Failed to get message")
                return
            resp = message[0]
            payload_size, msg_id = message[1]
            print("Received message: %d of size %d bytes"%(msg_id, payload_size))

            if (msg_id == ACK):
                print("Successfully sent button event")
            elif (msg_id == NACK):
                print("Error: command NACKed")
            else:
                print("Error: Unexpected command response")
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
                            await client.write_gatt_char(WRITE_UUID, data)
                    await asyncio.sleep(0.001)

                await client.stop_notify(WRITE_UUID)
        except exc.BleakError:
            print("Retrying connection...")
            connect_attempts -= 1
            await asyncio.sleep(0.100)
    if (connect_attempts == 0):
        print("Failed to connect...")


def ble_process(name, child_conn):
    global child_conn_g
    child_conn_g = child_conn
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.create_task(run_notification_and_name(address, loop, child_conn))
    loop.run_forever()







def worker():
    #Will get full messages from pipe (parent_conn), need to put into apprpriate queue
    while True:
        message = retrieveMessage(parent_conn)
        if (message != False):
            resp = message[0]
            payload_size, msg_id = message[1]
            msg_queue_dict[msg_id].put(resp)
        else:
            print("Failed to retrieve message")
            time.sleep(0.1)

def worker2(line):
    cnt = 0
    while True:
        print("Hello thread", cnt, line)
        cnt += 1
        time.sleep(5)

if __name__ == '__main__':
    parent_conn, child_conn = multiprocessing.Pipe()
    bt_process = multiprocessing.Process(target=ble_process, args=(1, child_conn))
    bt_process.start()

    for i in range(MSG_CMD_NUM):
        msg_queue_dict[i] = queue.Queue()

    line = 'cool'
    #threading.Thread(target=worker2, daemon=True, args=(line,)).start()
    threading.Thread(target=worker, daemon=True).start()

    POV_Shell().cmdloop()



    
