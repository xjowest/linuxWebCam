#include "webcam.h"

int main(void)
{
  int hCam = -1;
  struct v4l2_queryctrl ctrl;

  if(!initCam(&hCam)){
    printf("Could not open device\n");
    return -1;
  }

  if(!hasVideoCaptureCapability(hCam)){
    printf("Device has not video capture capabilities\n");
    return -1;
  }

  ioctl(hCam, VIDIOC_QUERYCTRL, &ctrl);
  
  if(!disposeCam(hCam)){
    printf("Could not close device\n");
    return -1;
  }

  return 0;
}

bool initCam(int * hCam)
{
  *hCam = open(deviceName, O_RDWR);
  if(*hCam == -1)
    return false;
  else
    return true;
}

bool disposeCam(int hCam)
{
  if(close(hCam) == -1)
    return false;
  else
    return true;
}

bool hasVideoCaptureCapability(int hCam)
{
  struct v4l2_capability camCap;
  memset(&camCap, 0, sizeof(camCap));
  ioctl(hCam, VIDIOC_QUERYCAP, &camCap);
  printf("Capabilities: %x\n", camCap.capabilities);
  return camCap.capabilities & V4L2_CAP_VIDEO_CAPTURE ? true : false;
}
 
