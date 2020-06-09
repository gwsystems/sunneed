# Edge
This directory contains scripts for edge detection testing. Edge detection can be used to figure out
how full the bin is. We can calculate surface area of the item while it is in the lid and then after it
falls into the bin. This provides information on how fall the item fell and thus how full the bin is.

## Virtual Env
Setup a virtual environment to isolate python dependencies from the system. Do not move the env directory, I think it messes up the path and python dependencies cannot be found.
1. `pip3 install virtualenv`
2. `python3 -m virtualenv env` Create environment
3. `source env/bin/activate` Activate the environment. It should be active when installing dependencies
and running python files.
4. `deactivate` when done.

## Install OpenCV and Dependencies
```
pip3 install opencv-python==3.4.6.27
pip3 install pillow
pip3 install matplotlib
pip3 install numpy

sudo apt-get install libatlas-base-dev
sudo apt-get install libjasper-dev
sudo apt-get install libqtgui4
sudo apt-get install python3-pyqt5
sudo apt-get install libqt4-test
```

## Set up the webcam
```
sudo apt install fswebcam
sudo usermod -a -G video your_user_name
sudo raspi-config (then enable camera and restart)
sudo apt install eog
```

## Python files
`image_operations.py` does image transformations using Python Image Library (PIL)
`canny.py` Canny operator (input a grayscale img and outputs edges using gradients)
`test2.py` Applies contours
`capture.py` Captures and saves a jpg, and opens it and canny in a window (TODO: most recent capture is not correct)
`test4.py` Automatically captures and saves image as jpg

