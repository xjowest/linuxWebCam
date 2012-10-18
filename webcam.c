#include "webcam.h"

struct{
  void *start;
  size_t length;
}*buffers;

int main(void)
{
  int hCam = -1;
  int i;
  enum v4l2_buf_type buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  u8 rgb_buf[320*240*3];
  SDL_Surface * screen;
  SDL_Surface * img;
  struct ImageSize imgSize;

  if(!initCam(&hCam)){
    printf("Could not open device\n");
    return -1;
  }

  if(!hasVideoCaptureCapability(hCam)){
    printf("Device doesn't have video capture capabilities\n");
    return -1;
  }

  if(!configureCamera(hCam)){
    printf("Could not configure the webcam, error %d\n", errno);
    return -1;
  }
  
  if(ioctl(hCam, VIDIOC_STREAMON, &buf_type) == -1)
    printf("Error streamon %u\n", errno);

  if(!fetchFrame(hCam)){
    printf("Could not fetch frame, error %u\n", errno);
    return -1;
  }

  convertFrame2RGB(rgb_buf);

  if(ioctl(hCam, VIDIOC_STREAMOFF, &buf_type) == -1)
    printf("Error streamoff %u\n", errno);
  

  for(i=0;i<REQ_BUF_COUNT;i++)
    munmap(buffers[i].start, buffers[i].length);
 
  if(!disposeCam(hCam)){
    printf("Could not close device\n");
    return -1;
  }

  SDL_Init(SDL_INIT_VIDEO);
  atexit(SDL_Quit);

  screen = SDL_SetVideoMode(320,240,24,SDL_HWSURFACE);

  img = SDL_CreateRGBSurfaceFrom(rgb_buf, 320, 240, 24, 
				 screen->pitch, 
				 screen->format->Rmask,
				 screen->format->Gmask,
				 screen->format->Bmask,
				 screen->format->Amask);
  
  SDL_BlitSurface(img, 0, screen, 0);
  SDL_Flip(screen);

  SDL_FreeSurface(screen);
  SDL_FreeSurface(img);
  SDL_Delay(10000);

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

bool configureCamera(int hCam)
{
  int i;
  struct v4l2_requestbuffers reqbuf;
  struct v4l2_buffer buf; 

  memset(&reqbuf, 0, sizeof(reqbuf));

  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  reqbuf.count = REQ_BUF_COUNT;

  if(ioctl(hCam, VIDIOC_REQBUFS, &reqbuf) == -1)
    return false;

  buffers = calloc(reqbuf.count, sizeof(*buffers));

  for(i=0;i<reqbuf.count;i++){
    memset(&buf, 0, sizeof(buf));  
    buf.type = reqbuf.type;
    buf.memory = reqbuf.memory;
    buf.index = i;

    if(ioctl(hCam, VIDIOC_QUERYBUF, &buf) == -1)
      return false;

    buffers[i].length = buf.length;
    buffers[i].start = mmap(NULL,
			    buf.length,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED,
			    hCam,
			    buf.m.offset);

    if(buffers[i].start == MAP_FAILED)
      return false;
  }

  for(i=0;i<reqbuf.count;i++){
    memset(&buf, 0, sizeof(buf));
    buf.type = reqbuf.type;
    buf.memory = reqbuf.memory;
    buf.index = i;

    if(ioctl(hCam, VIDIOC_QBUF, &buf) == -1)
      return false;
  }

  return true;
}

bool fetchFrame(hCam)
{
  static struct v4l2_buffer buf;   
  memset(&buf, 0, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  
  if(ioctl(hCam, VIDIOC_DQBUF, &buf) == -1)
    return false;

  return true;
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

void convertFrame2RGB(u8 * rgb_buf)
{
  int i;
  static struct RGB rgb1;
  static struct RGB rgb2;
  int indexYuv;
  int indexRgb;
  u8 * pixel_p = (u8*)buffers[0].start;

  for(i=0;i<buffers[0].length / 4 - 4;i++){
    indexYuv = i*4;
    indexRgb = i*6;

    rgb1 = YUV444toRGB(pixel_p[indexYuv + 0],
		       pixel_p[indexYuv + 1],
		       pixel_p[indexYuv + 3]);

    rgb2 = YUV444toRGB(pixel_p[indexYuv + 2],
		       pixel_p[indexYuv + 1],
		       pixel_p[indexYuv + 3]);

    rgb_buf[indexRgb+0] = rgb1.R;
    rgb_buf[indexRgb+1] = rgb1.G;
    rgb_buf[indexRgb+2] = rgb1.B;
    rgb_buf[indexRgb+3] = rgb2.R;
    rgb_buf[indexRgb+4] = rgb2.G;
    rgb_buf[indexRgb+5] = rgb2.B;
  }
}
  /*
    y1   = yuv[0];
    u    = yuv[1];
    y2   = yuv[2];
    v    = yuv[3];

    rgb1 = YUV444toRGB(y1, u, v);
    rgb2 = YUV444toRGB(y2, u, v);
  */

struct RGB YUV444toRGB(u8 y, u8 u, u8 v)
{
  int a, b, c;
  struct RGB rgb;
  
  a = y - 16;
  b = u - 128;
  c = v - 128;

  rgb.B = clamp((298*a + 409*c + 128) >> 8);
  rgb.G = clamp((298*a - 100*b - 208*c + 128) >> 8);
  rgb.R = clamp((298*a + 516*b + 128) >> 8);

  return rgb;
}

u8 clamp(int val)
{
  if(val < 0) val = 0;
  if(val > 255) val = 255;
  return (u8)val;
}


