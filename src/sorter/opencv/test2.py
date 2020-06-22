import cv2
import numpy as np

cap = cv2.VideoCapture(0)

while(1):
    _,frame=cap.read()
    # Blurring for removing the noise 
    img_blur = cv2.bilateralFilter(frame, d = 7, 
                                   sigmaSpace = 75, sigmaColor =75)
    # Convert to grayscale 
    img_gray = cv2.cvtColor(img_blur, cv2.COLOR_RGB2GRAY)
    # Apply the thresholding
    a = img_gray.max()  
    _, thresh = cv2.threshold(img_gray, a/2+60, a,cv2.THRESH_BINARY_INV)
    #plt.imshow(thresh, cmap = 'gray')

    # Find the contour of the figure 
    image, contours, hierarchy = cv2.findContours(
                                       image = thresh, 
                                       mode = cv2.RETR_TREE, 
                                       method = cv2.CHAIN_APPROX_SIMPLE)
    # Sort the contours 
    contours = sorted(contours, key = cv2.contourArea, reverse = True)
    # Draw the contour 
    img_copy = frame.copy()
    final = cv2.drawContours(img_copy, contours, contourIdx = -1, 
                             color = (255, 0, 0), thickness = 2)
    #plt.imshow(img_copy)

    hsv = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)
    lower_red = np.array([30,150,50])
    upper_red = np.array([255,255,180])

    mask = cv2.inRange(hsv, lower_red, upper_red)
    res = cv2.bitwise_and(frame,frame, mask= mask)

    cv2.imshow('Original',frame)
    edges = cv2.Canny(frame,100,200)
    #cv2.imshow('Edges',edges)

    cv2.imshow('Edges',img_copy)
    k = cv2.waitKey(5) & 0xFF
    if k == 27:
        break

cv2.destroyAllWindows()
cap.release()
