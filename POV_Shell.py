import sys, cmd, struct
import asyncio
from bleak import BleakClient
from bleak import exc
import threading
import time
import logging
import multiprocessing

GET_DISPLAY_SIZE =        0
GET_BUFFER_TYPE =         1
SET_BUFFER_TYPE =         2
CLEAR_DISPLAY =           3
UPDATE_DISPLAY =          4
GET_PERIOD =              5
BUTTON_EVENT =            6

address = 'F0:C7:7F:94:CC:6C'
NAME_UUID = "00002a00-0000-1000-8000-00805f9b34fb"
WRITE_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"

messages_to_display = []
messages_from_display = []
client = None
bt_process = None

parent_conn = None 
child_conn_g = None

class dummy():
    d = 0
    child_conn = None

dummy_class = dummy()

def notification_handler(sender, data):
    global child_conn
    global dummy_class
    #global messages_from_display
    #messages_from_display.append(data)
    print("Message received:")
    print("{0}: {1}".format(sender, data))
    print(child_conn_g)
    if (dummy_class.child_conn is None):
        print("Error: invalid pipe connection")
    else:
        dummy_class.child_conn.send(data)

def getMessage(message_id, payload):
    payload_size = len(payload)
    header = struct.pack('i i', payload_size, message_id)
    return (header + payload)


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
            global parent_conn
            send_data = bytearray(b'Test Send\n')
            parent_conn.send(send_data)
            data = bytearray(b'\x60\x00\x00\x00\x08\x00\x00\x00\x06\x00\x00\x00')
            
            if (parent_conn.poll(15)):
                resp_data = parent_conn.recv()
                print("Response:", resp_data)
            else:
                print("TIMEOUT waiting for response")
            

            (l, w, h) = struct.unpack('i i i', data) 
            print("Length: %d, Width: %d, Height: %d"%(l, w, h))
        except BaseException:
            print("Error:", sys.exc_info())

    def do_get_buffer_type(self, arg):
        'Returns whether single or double buffering is used'
        try:
            print("Double Buffered")
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


async def run_get_services(address, loop, parent_conn, child_conn):
    cnt = 0
    print("Establishing connection...")
    global client
    async with BleakClient(address, loop=loop) as client:
        print("Connected!")
        services = await client.get_services()

        while (True):
            print("parent_conn.poll(0):", child_conn.poll(0))
            if (child_conn.poll(0)):
                data = child_conn.recv() 
                print(data)
                await client.write_gatt_char(WRITE_UUID, data)
            print("Sleeping...")
            write_str = "Hello World: %d\n"%(cnt)
            write_bytes = bytearray()
            write_bytes.extend(write_str.encode())
            #test_bytes = convert_to_bytearray("Testing\n")

            await client.write_gatt_char(WRITE_UUID, write_bytes)
            '''
            await client.write_gatt_char(WRITE_UUID, set_colors(0, cnt%8, 0, 255, 0, 255))
            for i in range(96):
                for k in range(6):
                    await client.write_gatt_char(WRITE_UUID, set_colors(i, 3, k, 0, 255, 255))
            await client.write_gatt_char(WRITE_UUID, update())
            print("s %d %d %d %d %d %d\n"%(0, cnt%8, 0, 255, 0, 255))
            print("Packet Size:", len(clear()) + 96*6*len(set_colors(100, 100, 100, 100, 100, 100)) + len(update()))
            '''
            cnt += 1
            await asyncio.sleep(2.5)

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
                        data = child_conn.recv()
                        await client.write_gatt_char(WRITE_UUID, data)
                    await asyncio.sleep(0.01)
                #await asyncio.sleep(30.0)
                await client.stop_notify(WRITE_UUID)
        except exc.BleakError:
            print("Retrying connection...")
            connect_attempts -= 1
            await asyncio.sleep(0.100)
    if (connect_attempts == 0):
        print("Failed to connect...")


async def connect_test(address, loop, child_conn):
    connect_attempts = 3
    '''
    while (True):
        if (conn_request):
            conn_request = false
            establish_connection()
    '''

def thread_function(name, child_conn):
    #logging.info("Thread %s: starting", name)
    #time.sleep(2)
    #logging.info("Trhead %s: finishing", name)
    global dummy_class
    global child_conn_g
    child_conn_g = child_conn
    dummy_class.child_conn = child_conn
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.create_task(run_notification_and_name(address, loop, child_conn))
    loop.run_forever()
    
def thread_function2(name):
    POV_Shell().cmdloop()

def thread_function3(name):
    while(True):
        print("Thread", name)
        time.sleep(1)

if __name__ == '__main__':
    parent_conn, child_conn = multiprocessing.Pipe()
    bt_process = multiprocessing.Process(target=thread_function, args=(1, child_conn))
    #y = multiprocessing.Process(target=thread_function2, args=(1,))
    bt_process.start()
    #y.start()
    #y.join()
    #loop = asyncio.get_event_loop()
    #loop.create_task(run_get_services(address, loop))
    #loop.run_forever()
    POV_Shell().cmdloop()



    
