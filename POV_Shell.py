import sys, cmd, struct
import asyncio
from bleak import BleakClient
from bleak import exc
import threading
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

#Message Responses
ACK =                    0
NACK =                   1
GET_DISPLAY_SIZE_RESP =  2
GET_BUFFER_TYPE_RESP =   3
GET_PERIOD_RESP =        4
LOG_MSG =                5

address = 'F0:C7:7F:94:CC:6C'
NAME_UUID = "00002a00-0000-1000-8000-00805f9b34fb"
WRITE_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"

TIMEOUT_PERIOD = 15.0

messages_to_display = []
messages_from_display = []
client = None
bt_process = None

parent_conn = None 
child_conn_g = None

msg_footer = b'\xBE\xEF'

log_msg = bytearray()

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
    #print("Message received:")
    #print("{0}: {1}".format(sender, data))
    if (child_conn_g is None):
        print("Error: invalid pipe connection")
    else:
        #(payload_size, msg_id) = getMessageHeader(data)
        #if (msg_id == LOG_MSG):
        #print(getMessagePayload(data))
        print(data)
        #else:
        child_conn_g.send(data)


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
            message = retrieveMessage(parent_conn)
            if (message is False):
                print("Error: Failed to get message")
                return
            resp = message[0]
            payload_size, msg_id = message[1]
            print("Received message: %d of size %d bytes"%(msg_id, payload_size))
            payload = getMessagePayload(resp)
            print(payload)
            handled = True
            (l, w, h) = struct.unpack('i i i', payload) 
            print("Length: %d, Width: %d, Height: %d"%(l, w, h))
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
            payload = struct.pack('i', type)
            message = getMessage(SET_BUFFER_TYPE, payload)
            print(message)
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
            payload = struct.pack('i', update)
            message = getMessage(UPDATE_DISPLAY, payload)
            print(message)
        except BaseException:
            print("Error:", sys.exc_info())
    
    def do_update_display(self, arg):
        'Update display (switch front and rear buffers)'
        try:
            pass
        except BaseException:
            print("Error:", sys.exc_info())
    
    def do_get_period(self, arg):
        'Returns last recorded rotation period'
        try:
            print("20000 us")
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
            message = getMessage(BUTTON_EVENT, payload)
            print(message)
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
    connect_attempts = 3
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
                        data = bytearray(child_conn.recv())
                        #print("Sending:", data)
                        await client.write_gatt_char(WRITE_UUID, data)
                    await asyncio.sleep(0.001)

                await client.stop_notify(WRITE_UUID)
        except exc.BleakError:
            print("Retrying connection...")
            connect_attempts -= 1
            await asyncio.sleep(0.100)
    if (connect_attempts == 0):
        print("Failed to connect...")


async def connect_test(address, loop, child_conn):
    connect_attempts = 10
    '''
    while (True):
        if (conn_request):
            conn_request = false
            establish_connection()
    '''

def ble_process(name, child_conn):
    global child_conn_g
    child_conn_g = child_conn
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.create_task(run_notification_and_name(address, loop, child_conn))
    loop.run_forever()

if __name__ == '__main__':
    parent_conn, child_conn = multiprocessing.Pipe()
    bt_process = multiprocessing.Process(target=ble_process, args=(1, child_conn))
    bt_process.start()
    POV_Shell().cmdloop()



    
