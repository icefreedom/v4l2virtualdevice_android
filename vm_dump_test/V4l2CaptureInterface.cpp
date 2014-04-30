#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <assert.h>
#include <linux/version.h>
#include <pthread.h>
#include <sys/time.h>


#include "V4l2Capture.hpp"
#include "V4l2CaptureInterface.h"

#define ST_YUY2     41
#define ST_H264     43

#define YUY2_WIDTH  640
#define YUY2_HEIGHT 480
#define H264_WIDTH  640
#define H264_HEIGHT 480

#define REC_FRAMES 15*60 //10 sec
#define BITRATE 256 //2500KBPS
#define FPS 10 //30 FPS


#include <android/log.h>
#define THIS_FILE "V4l2CaptureInterface.cpp"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, THIS_FILE, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, THIS_FILE, __VA_ARGS__)
//#define LOGE(...)
//#define LOGD(...)

using namespace std;

struct ZXW_Capture {
    V4l2Capture *v4l2cap;
    int width;
    int height;
    int format;
};
//format: 1:YUV420, 2:h264
int zxw_capture_init(ZXW_Capture ** capture, int width, int height, int format, int devnum) {
    char DevicePath[32];
    sprintf(DevicePath,"/dev/video%d", devnum);
    LOGE("capture device path: %s", DevicePath);
    
    uint32_t pixFmt = V4L2_PIX_FMT_H264;
    switch(format) {
        case 1: pixFmt = V4L2_PIX_FMT_YUYV; break;
        case 2: pixFmt = V4L2_PIX_FMT_H264; break;
        default: pixFmt = 0; break;
    };
    ZXW_Capture *zxw_capture = (ZXW_Capture*) malloc(sizeof(ZXW_Capture));
    if (zxw_capture == NULL) {
        LOGE("memory error: fail to alloc ZXW_Capture");
        return -1;
    }
    zxw_capture->width = width;
    zxw_capture->height = height;
    zxw_capture->format = format;

    
    V4l2Capture *v4l2cap = new V4l2Capture(DevicePath);

    if(!v4l2cap->ConfigureDev(width, height,pixFmt, 0))
    {
        LOGE("Failed to config device.\r\n");
        v4l2cap->UnConfigureDev();
        return 0;
    }

    zxw_capture->v4l2cap = v4l2cap;

    *capture = zxw_capture;

    return 0;
}

int zxw_capture_deint(ZXW_Capture *capture) {
    if (capture == NULL) return 0;
    
    if (capture->v4l2cap != NULL ) {
        capture->v4l2cap->UnConfigureDev();
        delete capture->v4l2cap;
        capture->v4l2cap = NULL;
    }

    free(capture);
}

int zxw_capture_getFrame(ZXW_Capture *capture, char *frame, int *size) {
    if (capture == NULL || capture->v4l2cap == NULL) return  -1;
    capture->v4l2cap->CaptureFrame((uint8_t*)frame, (uint32_t*)size);
    return 0;
}

int zxw_capture_setH264Bitrate(ZXW_Capture *capture, int bitrate) {
    if (capture == NULL || capture->v4l2cap == NULL) return  -1;
    return capture->v4l2cap->SetH264BitRate(bitrate);
}
