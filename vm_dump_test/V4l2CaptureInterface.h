#ifndef __V4l2Capture_Interface_H__
#define __V4l2Capture_Interface_H__
#ifdef __cplusplus
extern "C" {
#endif
#define DEVICE_PATH "/dev/video2"

//#define V4L2_PIX_FMT_H264 v4l2_fourcc('H', '2', '6', '4')
#define V4L2_PIX_FMT_H264 0

typedef struct ZXW_Capture  ZXW_Capture;
//format: 1:YUV420, 2:h264
int zxw_capture_init(ZXW_Capture ** catpure, int width, int height, int format, int devnum);

int zxw_capture_deint(ZXW_Capture *capture);

int zxw_capture_getFrame(ZXW_Capture *capture, char *frame, int *size);

int zxw_capture_setH264Bitrate(ZXW_Capture *capture, int bitrate);


#ifdef __cplusplus
}
#endif

#endif
