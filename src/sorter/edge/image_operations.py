# -*- coding: utf-8 -*-
"""
Created on Fri Oct 18 10:51:46 2019

@author: Jonathan Lee
"""

# this part displays the image
from PIL import Image, ImageMath, ImageOps
import matplotlib.pyplot as plt

# Load an image
img = Image.open("colors.png")

# Show image
#plt.imshow(img)

# Add alpha channel
imAlpha = img.convert('RGBA')
# Convert to greyscale
imGreyscale = img.convert('L')

r,g,b,a = imAlpha.split() #split into r, g, b, a intensity images

# LOGICAL COLOR OPERATIONS:

# Intersection:
imIntersect = ImageMath.eval("convert(a&b,'L')", a=r, b=b) # red and blue 
imIntersect2 = ImageMath.eval("convert(a&~b,'L')", a=r, b=b) # red and not blue

# Isolate red channel:
a = ImageMath.eval("convert(r&a, 'L')", r=r,a=a) # makes only red opaque
g = ImageMath.eval("convert(g&0, 'L')", g=g) # make all green 0 intensity
b = ImageMath.eval("convert(b&0, 'L')", b=b) # make all blue 0 intensity
imRed = Image.merge("RGBA", (r,g,b,a)) # merge to get only red


###############################################################################
# this part removes the axis
def setup_figure(sz):
    # prep a figure with no axes
    fig = plt.figure(figsize=(sz[0],sz[1]))
    ax = plt.Axes(fig, [0., 0., 1., 1.])
    ax.margins(0,0)
    ax.axis('off')
    ax.xaxis.set_major_locator(plt.NullLocator())
    ax.yaxis.set_major_locator(plt.NullLocator())
    fig.add_axes(ax)
    return fig, ax

# Calculate dimensional information from the image
width, height = img.size
dpi = img.info["dpi"][0]
sz = [width/dpi, height/dpi]

# Use the setup_figure function to create an figure with no axes
fig, ax = setup_figure(sz)
###############################################################################

# Show the image
#plt.imshow(imAlpha) # original + alpha
#plt.imshow(r) # red split
#plt.imshow(imGreyscale, cmap = 'Greys_r') # greyscale
#plt.imshow(imIntersect) # red & blue
#plt.imshow(imIntersect, cmap = 'Greys_r') # red & blue greyscale
#plt.imshow(ImageOps.invert(imIntersect), cmap = 'Greys_r') # inverted red & blue + greyscale



#save file
plt.savefig("temp.png", bbox_inches='tight', pad_inches=0, transparent=True, dpi=dpi)