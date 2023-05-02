import board
import busio
import digitalio
import pwmio
from lib.PID import PID
from adafruit_hcsr04 import HCSR04
import time
from adafruit_motor.motor import DCMotor, SLOW_DECAY
# For most CircuitPython boards:
led = digitalio.DigitalInOut(board.LED)
network_ready = digitalio.DigitalInOut(board.D2)
# For QT Py M0:
# led = digitalio.DigitalInOut(board.SCK)
led.direction = digitalio.Direction.OUTPUT
network_ready.direction = digitalio.Direction.INPUT

uart = busio.UART(board.TX, board.RX, baudrate=115200, timeout=.1)
hcsr04 = HCSR04(board.D4, board.D3)

carPID = PID(0.01, 0, 0, 40, sample_time=None, output_limits=(-1, 1))
carPID.set_auto_mode(False) # wait to enable PID until connected

aIn1 = pwmio.PWMOut(board.D12)
aIn2 = pwmio.PWMOut(board.D11)
bIn1 = pwmio.PWMOut(board.D8)
bIn2 = pwmio.PWMOut(board.D9)
motSleep = digitalio.DigitalInOut(board.D10)
motSleep.direction = digitalio.Direction.OUTPUT
motSleep.value = False

leftMotor = DCMotor(aIn1, aIn2)
rightMotor = DCMotor(bIn1, bIn2)
leftMotor.decay_mode = SLOW_DECAY
rightMotor.decay_mode = SLOW_DECAY

runningMedian = []
MEDIAN_LENGTH = 3

def setMotorSpeed(newSpeed):
    # expects a value between -1 and 1
    # for now drive the two motors equally, might add drift correction w/ compass later
    leftMotor.throttle = newSpeed
    rightMotor.throttle = newSpeed

def computeRunningMedian(newVal):
    runningMedian.append(newVal)
    while len(runningMedian) > MEDIAN_LENGTH:
        runningMedian.pop(0)
    # now compute the median of the new list
    mid = len(runningMedian)//2
    sorted_median = runningMedian.copy()
    sorted_median.sort()
    return (sorted_median[mid] + sorted_median[-(mid+1)])/2

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
    motSleep.value = True # active low, so enable the motors

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
                    dataParsed = []
                    for inStr in data[2:].split(","):
                        # print(f"Now parsing inString: {inStr} into index: {len(dataParsed)}")
                        dataParsed.append(float(inStr))
                    # print(f"Finished parsing, data now: {dataParsed}")
                    carPID.tunings = dataParsed[0:3]
                    carPID.setpoint = dataParsed[3]
                    # reset the i term
                    carPID._integral = 0
                    print(f"Got new gains! Gains now: kP: {carPID.Kp}, kI: {carPID.Ki}, kD: {carPID.Kd}, setpoint: {carPID.setpoint}")
                else:
                    print("Command unknown")

        try:
            distance = hcsr04.distance # may delay up to .1s, by design
        except RuntimeError:
            distance = 400 # out of sensor range
        distance = computeRunningMedian(distance)
        
        output = carPID(distance)
        setMotorSpeed(output)

        uart.write(f"F:{distance},{distance-carPID.setpoint},{carPID.setpoint},{output},{time.monotonic()}".encode("ASCII"))
        # print(time.monotonic())
        
    print("Connection lost, waiting for next connection...")
    carPID.set_auto_mode(False) # disable PID until next connection so I term doesn't explode
    motSleep.value = False # active low, so disable the motors
    led.value = False