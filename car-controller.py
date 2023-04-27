import board
import busio
import digitalio
from lib.PID import PID
from adafruit_hcsr04 import HCSR04
import time

# For most CircuitPython boards:
led = digitalio.DigitalInOut(board.LED)
network_ready = digitalio.DigitalInOut(board.D2)
# For QT Py M0:
# led = digitalio.DigitalInOut(board.SCK)
led.direction = digitalio.Direction.OUTPUT
network_ready.direction = digitalio.Direction.INPUT

uart = busio.UART(board.TX, board.RX, baudrate=115200, timeout=.1)
hcsr04 = HCSR04(board.D4, board.D3)

carPID = PID(1, 0, 0, 40, sample_time=None, output_limits=(-1, 1))
carPID.set_auto_mode(False) # wait to enable PID until connected

while True:
    print("Waiting for a connection...")
    while not network_ready.value:
        pass
    print("connection found")
    led.value = True # TODO: use neopixel for status
    uart.write(b"\n \n \n") # purge old things that may still be in other side's buffer
    time.sleep(.5)
    uart.write(f'G:{carPID.Kp},{carPID.Ki},{carPID.Kd},{carPID.setpoint}'.encode("ASCII"))
    print(f"set gains: G:{carPID.Kp},{carPID.Ki},{carPID.Kd},{carPID.setpoint}")
    carPID.set_auto_mode(True)

    while network_ready.value:
        data = uart.readline()  # read in a websocket communication, if there is one
        # this will also delay by .1 seconds most of the time, by design

        if data is not None:
            try:
                data = data.decode("ASCII")
            except UnicodeError:
                pass # probably a mangled transmission
            else:
                #if the data's fine...
                print(f"Got data: {data}")  #

                if(data.startswith("G:")):
                    # TODO: parse new geins and setpoints into system
                    pass
                else:
                    print("Command unknown")

        try:
            distance = hcsr04.distance # may delay up to .1s, by design
        except RuntimeError:
            distance = 400 # out of sensor range
        
        output = carPID(distance)
        # TODO: set new motor speed

        uart.write(f"F:{distance},{distance-carPID.setpoint},{carPID.setpoint},{output},{time.monotonic()}".encode("ASCII"))
        # print(time.monotonic())
        
    print("Connection lost, waiting for next connection...")
    carPID.set_auto_mode(False) # disable PID until next connection so I doesn't explode