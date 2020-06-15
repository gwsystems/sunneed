'''
Take picture using keyboard input

'''

import time
print("Setting up")
start = time.time()

import os
import shutil
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

"""
Global Variables
"""
# width and height
# 160, 120
# 320, 240
# 640, 480
w = 640
h = 480
count = 0
pred = []
true = []
waste_types = ["paper", "plastic", "metal","cardboard","glass","trash"]
learn = load_learner(Path("/home/jay/fastai/data/"), 'export.pkl')
times = []
right = 0
wrong = 0
total = 0
""""""

end = time.time()
difference = end - start
print(difference)
print("Ready")

# take picture with width and height, and save as file name f to directory d
def picture(f):
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

# inference on file f, returns tensor
def inference(f):
    print("Running Inference")
    start = time.time()
    path = "/home/jay/fastai/test(" + str(w) + "," + str(h) + ")/" + f
    img = open_image(path)
    tensor = learn.predict(img)
    end = time.time()
    difference = end - start
    print(difference)
    #string = str(difference) + "\n"
    times.append(difference)
    return tensor

def parseTensor(t):
    s = str(t)
    list = s.split()
    return list
    
    
'''
main loop
|
V
'''

def getNext():
    char = ''
    while(char != 'c' and char != 'q'):
        char = input("Press 'c' to capture or 'q' to save and quit, then ENTER")
    return char

def getInput():
    actual = ""
    while(actual != "plastic" and actual != "paper" and actual != "cardboard" and actual != "trash" and actual != "glass" and actual != "metal"):
        actual = str(input("Manually enter item type:"))
    return str(actual)
    
def savePlot(true, pred):
    cm=confusion_matrix(true, pred, labels=waste_types)
    df_cm = pd.DataFrame(cm,waste_types,waste_types)
    plt.figure(figsize=(10,8))
    sns.heatmap(df_cm,annot=True,fmt="d",cmap="YlGnBu")
    #plt.show()
    path = str("/home/jay/fastai/" + "test(" + str(w) + "," + str(h) + ")/")
    name = str("matrix(" + str(w) + "," + str(h) + ").jpg")
    print("plot saved as " + name)
    fullPath = path + name
    plt.xlabel('Predicted')
    plt.ylabel('True')
    plt.savefig(fullPath)
    
def printStats():
    path = str("/home/jay/fastai/" + "test(" + str(w) + "," + str(h) + ")/stats.txt")
    stats = open(path,"w")
    accuracy = right/total
    #3 decimal places
    accuracy = round(accuracy,3)
    avg_time = sum(times)/len(times)
    avg_time = round(avg_time,3)
    line1 = "right: " + str(right) + "\n"
    line2 = "wrong: " + str(wrong) + "\n"
    line3 = "total: " + str(total) + "\n"
    line4 = str("ACCURACY: " + str(accuracy) + "\n")
    line5 = str("AVG TIME(s): " + str(avg_time) + "\n")
    stats.write(line1)
    stats.write(line2)
    stats.write(line3)
    stats.write(line4)
    stats.write(line5)
    timesStr = []
    for t in times:
        rounded = round(t,4)
        timesStr.append(str(rounded) + "\n")
    stats.writelines(timesStr)

while True:
    char = getNext()
    
    '''take a picture'''
    if (char == 'c'):
        # set file name and resolution
        filename = "trash" + str(count) + ".jpg"
        # take picture
        picture(filename)
        # inference
        tensor = inference(filename)
        # parseTensor
        identity = parseTensor(tensor)
        # add it to confusion matrix
        guess = identity[1]
        guess = guess[:-1]
        print(guess)
        pred.append(guess)
        actual = getInput()
        true.append(actual)
        # move image to correct or incorrect folder
        path = str("/home/jay/fastai/" + "test(" + str(w) + "," + str(h) + ")/")
        absoluteFile = path + filename
        correct = path + "correct"
        incorrect = path + "incorrect"
        if (guess == actual):
            shutil.move(absoluteFile,correct)
            right += 1
            total += 1
        else:
            shutil.move(absoluteFile,incorrect)
            wrong += 1
            total += 1
        print(confusion_matrix(true, pred, labels=waste_types))
        count += 1
        
    '''save and quit'''
    if (char == 'q'):
        savePlot(true, pred)
        printStats()
        break
    
