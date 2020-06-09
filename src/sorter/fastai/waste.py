import time

print("importing")
start = time.time()

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
print("starting inference")
start = time.time()


### Print versions
#import torch
#print(torch.__version__)
#import fastai
#print(fastai.__version__)

#learn = create_cnn(data,models.resnet34,metrics=error_rate)
#learn = load_learner("/home/jay/fastai/Waste-Sorter/data/export.pkl")

learn = load_learner(Path("/home/jay/fastai/data/"), 'export.pkl')
img = open_image("/home/jay/fastai/test3.jpg")
print(learn.predict(img))

end = time.time()
difference = end - start
print(difference)
