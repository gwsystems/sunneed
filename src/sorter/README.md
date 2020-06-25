# Recycling Sorter
A smart 2-in-1 recycling and trash bin that automatically sorts recyclables from
trash. This system provides a embedded application for sunneed. It uses a
raspberry pi with a webcam and servos to sort recyclables from trash. Installed
around cities, it reduces recycling contamination by removing human error from
the process.

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

## Machine Learning Frameworks

## Fastai
Based off Pytorch.

## Tensorflow (Lite)


## Neural Net Architectures
There are many neural networks architecture. Each implement different techniques and configurations.
Generally, the more layers the network has, the more accurate it becomes, but it requires more
computation power, such as GPUs. Once the model is trained, inference is done on either using cpu, gpu, or tpu (tensor processing unit).

Finding the best architecture for this project takes some trial and error. It needs to be lightweight,
fast, low power, and accurate.

### Resnet34
The first architecture I tested. It classifies images into glass, plastic, metal, paper, cardboard, and trash.
Initial testing accuracy of 50-60% with an inference time of ~2 seconds.

### MobileNetv2
A neural network designed to be deployed on devices with low computing power (cell phones, tablets, etc).

### Other architectures
- Detectron2
- Inception v2
- DeepLabv3, semantic segmentation
- YOLOv3
## scr
Each identification technique has its own subdirectory and python virtual environment
- edge: contains py files for image segmentation
- fastai: contains py files for inference and classification

## The Physical System
- Preliminary testing will modify an existing swing-lid style trash can with
  two bags, one under each flap.
- Eventually powered of solar and battery
