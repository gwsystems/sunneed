import os

os.system('git clone -b isochamber https://github.com/gwsystems/sunneed')
os.system('python3 build.py')
os.system('python3 testscript.py')