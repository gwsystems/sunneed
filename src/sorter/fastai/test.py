import time
print("importing")
start = time.time()

import pygame
import pygame.camera
from pygame.locals import *

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

end = time.time()
difference = end - start
print(difference)
print("done importing")

print("taking picture")
pygame.init()
pygame.camera.init()
cam = pygame.camera.Camera("/dev/video0",(640,480))
print(cam.get_size)
cam.start()
image = cam.get_image()
image = pygame.transform.scale(image, (160,120))
#(640,480) | 9.15s | 4:3 aspect ratio
#(512,384) | 5.49s | 4:3
#(320,240) | 2.54s | 4:3
#(160,120) | 1.08s | 4:3
#(256,144) | 7.98s | 19:6 aspect ratio
#(209,476) | 0.95s | random ratio

pygame.image.save(image,"test3.jpg")

print("starting inference")
start = time.time()
learn = load_learner(Path("/home/jay/fastai/data/"), 'export.pkl')
img = open_image("/home/jay/fastai/test3.jpg")
print(learn.predict(img))

end = time.time()
difference = end - start
print(difference)



