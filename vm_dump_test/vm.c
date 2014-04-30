#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>           
#include <fcntl.h>             
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>         
#include <linux/videodev2.h>

//#include "vmioctrl.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))
static int frame_count = 1000;
struct buffer {
        void *                  start;
        size_t                  length;
};

static char *           dev_name        = "/dev/video1";//default device name
static int              fd              = -1;
struct buffer *         buffers         = NULL;
static unsigned int     n_buffers       = 0;
const static short img_width = 640;
const static short img_height = 480;
static int *file_fd = -1;
static unsigned char *file_name;
//////////////////////////////////////////////////////
//Get frame
//////////////////////////////////////////////////////
static int read_frame (void)
{
	struct v4l2_buffer buf;
	unsigned int i;
	
	CLEAR (buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	
	ioctl (fd, VIDIOC_DQBUF, &buf); //Get frame from buffer

	
	
	 write(file_fd, buffers[buf.index].start,buf.bytesused);	//Dump video to a file
	ioctl (fd, VIDIOC_QBUF, &buf); //re-queue buffer
	
	return 1;
}

int main (int argc,char ** argv)
{
	struct v4l2_capability cap; 
	struct v4l2_format fmt;
	unsigned int i;
	enum v4l2_buf_type type;
	struct vdIn *m_vd;


	
	file_fd = open("test-mmap0.es", O_CREAT|O_WRONLY|O_NONBLOCK, S_IRWXU|S_IRWXG|S_IRWXO);
	fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
	
	
	ioctl (fd, VIDIOC_QUERYCAP, &cap);//get device capability
	
	CLEAR (fmt);
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = img_width;
	fmt.fmt.pix.height      = img_height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;//V4L2_PIX_FMT_YUYV;//
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;
	ioctl (fd, VIDIOC_S_FMT, &fmt); 												//set video format. 
	
	
	
	struct v4l2_requestbuffers req;
	CLEAR (req);
	req.count               = 4;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;
	
	ioctl (fd, VIDIOC_REQBUFS, &req); //request buffers from V4L2
	
	if (req.count < 2)
	   printf("Insufficient buffer memory\n");
	
	buffers = calloc (req.count, sizeof (*buffers));				//alloca buffers
	
	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
	{
	   struct v4l2_buffer buf;   
	   CLEAR (buf);
	   buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	   buf.memory      = V4L2_MEMORY_MMAP;
	   buf.index       = n_buffers;
	
	   if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf)) //ӳ
	    printf ("VIDIOC_QUERYBUF error\n");
	    
	   buffers[n_buffers].length = buf.length;
	   buffers[n_buffers].start =
	   mmap (NULL 															/* start anywhere */,    //mmap memory
	    buf.length,
	    PROT_READ | PROT_WRITE 												/* required */,
	    MAP_SHARED 															/* recommended */,
	    fd, buf.m.offset);
	
	   if (MAP_FAILED == buffers[n_buffers].start)
	   		printf ("mmap failed\n");
	}
	
	for (i = 0; i < n_buffers; ++i) 
	{
	   struct v4l2_buffer buf;
	   CLEAR (buf);
	
	   buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	   buf.memory      = V4L2_MEMORY_MMAP;
	   buf.index       = i;
	
	   if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))							
	   		printf ("VIDIOC_QBUF failed\n");
	}
	                
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (-1 == ioctl (fd, VIDIOC_STREAMON, &type)) 
	   printf ("VIDIOC_STREAMON failed\n");
	fd_set fds;
	for (int i = 0; i < frame_count; i++) 
	{
	  
	   struct timeval tv;
	   int r;
	
	   FD_ZERO (&fds);					
	   FD_SET (fd, &fds);				
	
	   /* Timeout. */
	   tv.tv_sec = 1;
	   tv.tv_usec = 0;
	
	   r = select (fd + 1, &fds, NULL, NULL, &tv);
	
	   if (read_frame ())
	   		printf("R\n");
	   //break;
	}
	
	unmap:
	for (i = 0; i < n_buffers; ++i)
	   if (-1 == munmap (buffers[i].start, buffers[i].length))
	    printf ("munmap error");
	close (fd);
	close (file_fd);
	free(buffers);
	exit (EXIT_SUCCESS);
	return 0;
}
