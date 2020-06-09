# Recycling Sorter
A smart 2-in-1 recycling and trash bin that automatically sorts recyclables from
trash. This system provides a embedded application for sunneed. It uses a 
raspberry pi with a webcam and servos to sort recyclables from trash. Installed
around cities, it reduces recycling contamination by removing human error from
the system. 

## Documentation
Google Doc for the installation process on Raspberry Pi:
https://docs.google.com/document/d/1e_PN6ShMLrPp-xht5e6tAIZ53sMQi1K1XGqgeyfKUgE/edit?usp=sharing
Google Doc for brainstorming process

## Computer Vision
Testing of various CV techniques.
### Fastai
About 50 to 60% inference accuracy upon initial testing using resnet-34 with 
6 categories: plastic, metal, cardboard, paper, glass, and trash.
### OpenCV
Use image segmentation to determine shape and surface area. Might be able to
make depth calculations.
### Tensorflow/Tensorflow Lite

### MobileNetv2

## scr
Each identification technique has its own subdirectory and virtual environment
- edge: contains py files for image segmentation
- fastai: contains py files for inference and classification
 
## The Physical System
- Preliminary testing will modify an existing swing-lid style trash can with 
  two bags, one under each flap.
- Eventually powered of solar and battery


