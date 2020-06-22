import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BOARD)
GPIO.setup(3,GPIO.OUT)
pwm= GPIO.PWM(3,50)
pwm.start(0)

def SetAngle(angle):
    duty = angle /20
    GPIO.output(3,True)
    pwm.ChangeDutyCycle(duty)
    sleep(1)
    GPIO.output(3,False)
    pwm.ChangeDutyCycle(0)

SetAngle(90)
SetAngle(0)
SetAngle(180)
text = input("enter angle:")
text = int(text)
if (text > -1 and text < 181):
    SetAngle(text)
pwm.stop()
GPIO.cleanup()
