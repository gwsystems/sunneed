import RPi.GPIO as GPIO
import time

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(10,GPIO.IN,pull_up_down=GPIO.PUD_UP) #initial state is low

pressed = False
count = 0
while True:
    if (GPIO.input(10) == GPIO.LOW and pressed == False):
        filename = "test" + str(count)
        print(filename)
        count = count + 1
        pressed = True
        time.sleep(0.25) # delay in seconds
    if (GPIO.input(10) == GPIO.HIGH):
        pressed = False
