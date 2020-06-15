## Virtual Env
Setup a virtual environment to isolate python dependencies from the system.
1. `pip3 install virtualenv`
2. `python3 -m virtualenv env` Create environment
3. `source env/bin/activate` Activate the environment. It should be active when installing dependencies
and running python files.
4. `deactivate` when done.

## Install OpenCV and Dependencies
See requirements.txt for other dependencies that need to be installed.
```
pip3 install opencv-python==3.4.6.27
pip3 install pygame
pip3 install RPi.GPIO
pip3 install wiringpi
sudo apt install python3-sklearn
```
If openCV doesn't work, try `sudo apt install python-opencv`

## Set up the webcam
```
sudo apt install fswebcam
sudo usermod -a -G video your_user_name
sudo raspi-config (then enable camera and restart)
sudo apt install eog
```

## Run
`python jupyter.py to train a new model (Would not recomend. Takes over 24 hrs to complete on the Pi 4. Use export.pkl instead)
`python waste.py` to do inference on sample image
`python webcam.py` to take an image using the webcam
 

