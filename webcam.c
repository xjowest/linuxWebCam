#include "webcam.h"

int main(void)
{
  int hCam = -1;
  int i;
  struct v4l2_requestbuffers reqbuf;
 
  struct{
    void *start;
    size_t length;
  }*buffers;

   memset(&reqbuf, 0, sizeof(reqbuf));

  if(!initCam(&hCam)){
    printf("Could not open device\n");
    return -1;
  }

  if(!hasVideoCaptureCapability(hCam)){
    printf("Device doesn't have video capture capabilities\n");
    return -1;
  }
  
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  reqbuf.count = 5;

  if(ioctl(hCam, VIDIOC_REQBUFS, &reqbuf) == -1)
    printf("Error %d\n", errno);

  buffers = calloc(reqbuf.count, sizeof(*buffers));

  for(i=0;i<reqbuf.count;i++){
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));  
    buf.type = reqbuf.type;
    buf.memory = reqbuf.memory;
    buf.index = i;

    if(ioctl(hCam, VIDIOC_QUERYBUF, &buf) == -1)
      printf("Error %d\n", errno);

    buffers[i].length = buf.length;
    buffers[i].start = mmap(NULL,
			    buf.length,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    hCam,
			    buf.m.offset);

    if(buffers[i].start == MAP_FAILED)
      printf("Error %u\n", errno);
  }
  
  /*
    y1   = yuv[0];
    u    = yuv[1];
    y2   = yuv[2];
    v    = yuv[3];

    rgb1 = YUV444toRGB(y1, u, v);
    rgb2 = YUV444toRGB(y2, u, v);
  */
 
  for(i=0;i<reqbuf.count;i++)
    munmap(buffers[i].start, buffers[i].length);
 
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
  return camCap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING) ? true : false;
}
 
struct ImageSize getImageSize(int hCam)
{
  struct v4l2_format format;
  struct ImageSize imageSize;
  
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  ioctl(hCam, VIDIOC_G_FMT, &format);
  ioctl(hCam, VIDIOC_S_FMT, &format);
  
  imageSize.width = format.fmt.pix.width;
  imageSize.height = format.fmt.pix.height;
        
  return imageSize;
}

struct RGB YUV444toRGB(u8 y, u8 u, u8 v)
{
  int a, b, c;
  struct RGB rgb;
  
  a = y - 16;
  b = u - 128;
  c = v - 128;

  rgb.R = clamp((298*a + 409*c + 128) >> 8);
  rgb.G = clamp((298*a - 100*b - 208*c + 128) >> 8);
  rgb.B = clamp((298*a + 516*b + 128) >> 8);

  return rgb;
}

u8 clamp(int val)
{
  if(val < 0) val = 0;
  if(val > 255) val = 255;
  return (u8)val;
}


