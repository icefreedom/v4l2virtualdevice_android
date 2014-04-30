#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#define DEVICE_PATH "/dev/video0"

//#ifndef V4L2_PIX_FMT_H264
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
//#define V4L2_PIX_FMT_H264 v4l2_fourcc('H', '2', '6', '4')
//#else
#define V4L2_PIX_FMT_H264 0 //remake for new kernel
//#endif
//#endif

#define ST_YUY2     41
#define ST_H264     43

#define YUY2_WIDTH  640
#define YUY2_HEIGHT 480
#define H264_WIDTH  640
#define H264_HEIGHT 480

#define REC_FRAMES 15*60 //10 sec
#define BITRATE 256 //2500KBPS
#define FPS 10 //30 FPS

using namespace std;

bool compute_again = true;
struct timeval start_time, current_time;
int yuv_count = 0;
bool exit_flag = true;

static int my_count = 0;


int main(int argc,char** argv)
{
    //thread_param_set();
    bool exit = false;
    char DevicePath[32];
    int interval;

    if(argc>1)
    {
        sprintf(DevicePath,"%s",argv[1]);
    }
    else
    {
        sprintf(DevicePath,DEVICE_PATH,argv[1]);
    }
    printf("DevicePath = %s\n", DevicePath);

    V4l2Capture *v4l2cap = new V4l2Capture(DevicePath);

    if(!v4l2cap->ConfigureDev(H264_WIDTH,H264_HEIGHT,V4L2_PIX_FMT_YUYV, 0))
    {
        printf("Failed to config device.\r\n");
        v4l2cap->UnConfigureDev();
        return 0;
    }

	MediaSample sample;
        sample.AllocBuffer(H264_WIDTH*H264_HEIGHT*2);

	FILE *fd_h264 = NULL;
	//FILE *fd_yuy2 = NULL;
	fd_h264 = fopen("yuy2_dump.bin","w+b");
	//fd_yuy2 = fopen("yuy2_dump.bin","w+b");

        exit_flag = false;
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
	//for(int n =0; n<REC_FRAMES+12; ++n)
    int n = 0;
    for(;;)
	{
	       uint32_t frame_size = v4l2cap->CaptureFrame(sample);

               if(frame_size>0)
               {
                    //fwrite(sample.GetBuffer(), frame_size, 1, fd_h264);
                         //fwrite( media_sample.yuy2, media_sample.yuy2_size, 1, fd_yuy2);
               }
               else
               {
                 printf("frame size = 0");
               }

            if (n % 50 == 0) {
                if (n == 0) {
                    gettimeofday(&start, NULL);
                } else {
                gettimeofday(&end, NULL);
                long secs = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
                secs /= 1000000;
                printf("fps = %f\n", 50.0 / secs);
                }
                gettimeofday(&start, NULL);
            }
            ++n;

          }
//		printf("frame %d\r\n",n);

        printf("capture over\n");
    //if(fd_yuy2)
    //    fclose(fd_yuy2);
	if(fd_h264)
        fclose(fd_h264);

	//close uvc camera
	v4l2cap->UnConfigureDev();
	delete v4l2cap;

        exit_flag = true;

        return 0;
}
