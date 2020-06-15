"""
import time
from SimpleCV import Camera

cam = Camera()
time.sleep(0.1)  # If you don't wait, the image will be dark
img = cam.getImage()
img.save("simplecv.png")
"""
"""
import pygame
import pygame.camera

pygame.camera.init()
cam = pygame.camera.Camera(0,(640,480))
cam.start()
img = cam.get_image()
pygame.image.save(img,"filename.jpg")
"""

import pygame
import pygame.camera
from pygame.locals import *

pygame.init()
pygame.camera.init()

cam = pygame.camera.Camera("/dev/video0",(640,480))
print(cam.get_size)
cam.start()
image = cam.get_image()
image = pygame.transform.scale(image, (512,384))
pygame.image.save(image,"test3.jpg")
