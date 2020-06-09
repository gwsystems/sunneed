'''
Take picture using button attached to GPIO pin 10
changes position of servo using setAngle
'''
import time
print("Setting up")
start = time.time()

import os
import RPi.GPIO as GPIO
import pygame
import pygame.camera
from pygame.locals import *
from time import sleep

from fastai.vision import *
from fastai.metrics import error_rate
from pathlib import Path
from glob2 import glob
from sklearn.metrics import confusion_matrix

import pandas as pd
import numpy as np
import os
import zipfile as zf
import shutil
import re
import seaborn as sns
import matplotlib.pyplot as plt

'''
Global Variables
'''
# width and height
w = 160
h = 120

''''''


GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(10,GPIO.IN,pull_up_down=GPIO.PUD_UP) #initial state is low
GPIO.setup(3,GPIO.OUT)
pwm= GPIO.PWM(3,50)
pwm.start(0)

end = time.time()
difference = end - start
print(difference)
print("Ready")

#(640,480) | 9.15s | 4:3 aspect ratio
#(512,384) | 5.49s | 4:3
#(320,240) | 2.54s | 4:3
#(160,120) | 1.08s | 4:3
#(256,144) | 7.98s | 19:6 aspect ratio
#(209,476) | 0.95s | random ratio

# take picture with width and height, and save as file name f to directory d
def picture(w,h,f):
    pygame.init()
    pygame.camera.init()
    cam = pygame.camera.Camera("/dev/video0",(640,480))
    print(cam.get_size)
    cam.start()
    print("Capturing " + str(f) + " with resolution " + str(w) + " x " + str(h))
    image = cam.get_image()
    image = pygame.transform.scale(image, (w,h)) #160,120
    path = "/home/jay/fastai/test(" + str(w) + "," + str(h) + ")/" + f
    pygame.image.save(image,path) # ex "trash0.jpg"
    cam.stop()
    del(cam)

# inference on file f
def inference(f):
    print("Running Inference")
    start = time.time()
    learn = load_learner(Path("/home/jay/fastai/data/"), 'export.pkl')
    path = "/home/jay/fastai/test(" + str(w) + "," + str(h) + ")/" + f
    img = open_image(path)
    #print(learn.predict(img))
    end = time.time()
    difference = end - start
    print(difference)
    return learn.predict(img)

def parseTensor(t):
    #list = [""] * 9
    s = str(t)
    list = s.split()
    #print(list)
    return list
    
    
# sets servo to angle from 0 to 180 degrees
def setAngle(angle):
    duty = angle /18+2
    GPIO.output(3,True)
    pwm.ChangeDutyCycle(duty)
    sleep(1)
    GPIO.output(3,False)
    pwm.ChangeDutyCycle(0)
    
# main loop
pressed = False
count = 0
materials = ["paper", "cardboard", "metal", "plastic", "glass", "trash"]
position = [30,60,90,120,150,180]
pred = []
true = []

def getInput():
    actual = input("Manually enter item (ie 'plastic'):")
    return str(actual)

def savePlot(true, pred):
    waste_types = ["paper", "plastic", "metal","cardboard","glass","trash"]
    cm=confusion_matrix(true, pred, labels=waste_types)
    df_cm = pd.DataFrame(cm,waste_types,waste_types)
    plt.figure(figsize=(10,8))
    sns.heatmap(df_cm,annot=True,fmt="d",cmap="YlGnBu")
    plt.show()
    name = str("matrix(" + str(w) + "," + str(h) + ").jpg") 
    plt.savefig(name)

while True:
    if (GPIO.input(10) == GPIO.LOW and pressed == False):
        # set file name and resolution
        filename = "trash" + str(count) + ".jpg"
        # take picture
        picture(w,h,filename)
        # inference
        tensor = inference(filename)
        # parseTensor
        identity = parseTensor(tensor)
        # add it to confusion matrix
        string = identity[1]
        string = string[:-1]
        print(string)
        pred.append(string)
        true.append(getInput())
        print(confusion_matrix(true, pred, labels=["paper", "plastic", "metal","cardboard","glass","trash"]))
        #set the servo position
        i = 0
        for category in materials:
            if category == string:
                setAngle(position[i])
            i = i + 1
        count = count + 1
        pressed = True
        time.sleep(0.25) # delay in seconds
    if (GPIO.input(10) == GPIO.HIGH):
        pressed = False
    
