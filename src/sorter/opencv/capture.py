import cv2
import numpy as np

# global vars
cap = cv2.VideoCapture(0) # webcam

def getInput():
    char = ""
    while(char != 'c'):
        char = input("Press 'c' to capture")
    return True

def getDist():
    x = 0
    while(x == 0):
        x = input("Enter initial distance")
    return x

def capImg():
    _,frame=cap.read()
    hsv = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)
    lower_red = np.array([30,150,50])
    upper_red = np.array([255,255,180])

    mask = cv2.inRange(hsv, lower_red, upper_red)
    res = cv2.bitwise_and(frame,frame, mask= mask)

    cv2.imshow('Original',frame)
    edges = cv2.Canny(frame,100,200)
    cv2.imshow('Edges',edges)
    k = cv2.waitKey(5) & 0xFF

def capImg2():
    s, img = cap.read()
    if s:    # frame captured without any errors
        cv2.imshow("filename.jpg",img)
        edges = cv2.Canny(img,100,200)
        cv2.imshow('Edges',edges)
        # press any key on window to close
        cv2.waitKey(0)
        cv2.destroyAllWindows()
        #destroyWindow("cam-test")
        cv2.imwrite("filename.jpg",img) #save image

    
'''
- take picture of empty background
- add item and take picture
- input distance of item
- compare images and output a difference image
- clean image
- calculate surface area in pixels
- move item and recalculate surface area
- calculate and return distance

'''

while(1):
    getInput()
    capImg2()
'''
while(1):
    _,frame=cap.read()
    hsv = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)
    lower_red = np.array([30,150,50])
    upper_red = np.array([255,255,180])

    mask = cv2.inRange(hsv, lower_red, upper_red)
    res = cv2.bitwise_and(frame,frame, mask= mask)

    cv2.imshow('Original',frame)
    edges = cv2.Canny(frame,100,200)
    cv2.imshow('Edges',edges)

    k = cv2.waitKey(5) & 0xFF
    if k == 27:
        break
'''
cv2.destroyAllWindows()
cap.release()
