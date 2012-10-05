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

static const char * deviceName = "/dev/video0";

typedef enum {false,true}bool;

bool initCam(int * hCam);
bool disposeCam(int hCam);
bool hasVideoCaptureCapability(int hCam);


