#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#define RGB_FRAME_SIZE 640*480*3
#define YUV_FRAME_SIZE 640*480*2
#define FRAME_SIZE 640*480
#define REQ_BUF_COUNT 5

static const char * deviceName = "/dev/video0";

typedef unsigned int u32;
typedef unsigned char u8;
typedef enum {false,true}bool;

struct ImageSize{
  u32 width;
  u32 height;
};

struct RGB{
  u8 R;
  u8 G;
  u8 B;
};

struct YUV{
  u8 Y;
  u8 U;
  u8 V;
};    

bool initCam(int * hCam);
bool disposeCam(int hCam);
bool hasVideoCaptureCapability(int hCam);
bool configureCamera(int hCam);
bool fetchFrame(int hCam);
void convertFrame2RGB(u8 * rgb_buf);
struct ImageSize getImageSize(int hCam);
struct RGB YUV444toRGB(u8 y, u8 u, u8 v);
u8 clamp(int val);



