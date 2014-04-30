/*
 * Copyright (C) 2007-2009 Skype Technologies S.A. Confidential and proprietary
 *
 * All intellectual property rights, including but not limited to copyrights,
 * trademarks and patents, as well as know how and trade secrets contained
 * in, relating to, or arising from the internet telephony software of Skype
 * Limited (including its affiliates, "Skype"), including without limitation
 * this source code, Skype API and related material of such software
 * proprietary to Skype and/or its licensors ("IP Rights") are and shall
 * remain the exclusive property of Skype and/or its licensors. The recipient
 * hereby acknowledges and agrees that any unauthorized use of the IP Rights
 * is a violation of intellectual property laws.
 *
 * Skype reserves all rights and may take legal action against infringers of
 * IP Rights.
 *
 * The recipient agrees not to remove, obscure, make illegible or alter any
 * notices or indications of the IP Rights and/or Skype's rights and ownership
 * thereof.
 */
#include "V4l2Capture.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <linux/uvcvideo.h>
#include <assert.h>

#undef LOG_PREFIX
#define LOG_PREFIX "V4l2Capture:"

#ifdef ANDROID
#include <android/log.h>
#define THIS_FILE "V4l2Capture.cpp"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, THIS_FILE, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, THIS_FILE, __VA_ARGS__)
#else
#define LOGE(...)
#define LOGD(...)
#endif
#include "debugging.hpp"

#define CAP_DBG DBG

#define NB_BUFFER 16



#define V4L2_CTRL_CLASS_VM          0x00dd0000
#define V4L2_CID_VM_EXT_BASE            (V4L2_CTRL_CLASS_VM | 0x900)    




//PROPSETID_VM_XU_CONTROL
//a917c75d-4119-11da-ae0e000d56ac7b4c

#define PROPSETID_VM_XU_CONTROL { \
    0x5d, 0xc7, 0x17, 0xa9, 0x19, 0x41, 0xda, 0x11, \
    0xae, 0x0e, 0x00, 0x0d, 0x56, 0xac, 0x7b, 0x4c  \
}

#define UVC_GUID_VM_EXT { \
    0xbb, 0xae, 0xde, 0xe3, 0xec, 0xc2, 0x42, 0x4d, \
    0x80, 0x2c, 0x72, 0xf2, 0xeb, 0x3d, 0xfb, 0x57  \
}

#define GUID_SIZE       16


#define EU_VMH264_CONTROL       0x1A

 #define UVC_SET_CUR                                     0x01
 #define UVC_GET_CUR                                     0x81
 #define UVC_GET_MIN                                     0x82
 #define UVC_GET_MAX                                     0x83
 #define UVC_GET_RES                                     0x84
 #define UVC_GET_LEN                                     0x85
 #define UVC_GET_INFO                                    0x86
 #define UVC_GET_DEF                                     0x87

struct FrameBuf {
	void *start;
	size_t length;
};

V4l2Capture::V4l2Capture(const char *devName) :
	m_pFrameBuf(NULL),
	m_pDevName(devName),
        m_IsConfigured(0),
	m_NBuffers(0)
{
	OpenDevice();
}

V4l2Capture::~V4l2Capture()
{
	if (m_IsConfigured)
		UnConfigureDev();

	CloseDevice();
}

#define CLEAR(x) memset (&(x), 0, sizeof (x))

static int xioctl(int fd, int request, void * arg)
{
	int r;

	do {
		r = ioctl(fd, request, arg);
	} while (-1 == r && EINTR == errno);
	return r;
}

void V4l2Capture::OpenDevice()
{
	struct stat st;

	if (-1 == stat(m_pDevName, &st)) {
		LOGD("Cannot identify '%s': %d, %s\n", m_pDevName, errno,
				strerror(errno));
		FATAL("Cannot identify '%s': %d, %s\n", m_pDevName, errno,
				strerror(errno));
	}

	if (!S_ISCHR(st.st_mode)) {
		LOGD("%s is no device\n", m_pDevName);
		FATAL("%s is no device\n", m_pDevName);
	}

	m_Fd = open(m_pDevName, O_RDWR /* required *//*| O_NONBLOCK*/, 0);
	//m_Fd = open(m_pDevName, O_RDWR , 0);

	if (-1 == m_Fd) {
		LOGD("Cannot open '%s': %d, %s\n", m_pDevName, errno,
				strerror(errno));
		FATAL("Cannot open '%s': %d, %s\n", m_pDevName, errno,
				strerror(errno));
	}
}

void V4l2Capture::CloseDevice()
{
    if(m_Fd<0)
        return;

	if (-1 == close(m_Fd))
		FATAL("close");

	m_Fd = -1;
}


//#include "IAitH264Cam.h"
bool V4l2Capture::SetFramerate(int fps)
{/*
	struct v4l2_streamparm parm;
	memset(&parm, 0, sizeof(parm));
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	// first get current values
	if (xioctl(m_Fd, VIDIOC_G_PARM, &parm) == 0) {

		DBG("current framerate params: numerator %d denominator, %d", parm.parm.capture.timeperframe.numerator,
				parm.parm.capture.timeperframe.denominator);

		DBG("SetFramerate VIDIOC_G_PARM: %d", fps);

		// change rate
		// time per frame is numerator / denominator in seconds
		parm.parm.capture.timeperframe.numerator = 1;
		parm.parm.capture.timeperframe.denominator = fps;

		// set new values
		if(-1 == xioctl(m_Fd, VIDIOC_S_PARM, &parm)) {
			ERROR("V4l2Capture: Failed to set streamparm: %s\n", strerror(errno));
			return false;
		}
	// now comes the special hack for pwc driver...
	// http://osdir.com/ml/drivers.pwc/2005-11/msg00001.html
	} else {
		DBG("V4l2Capture: Failed to get streamparm: %s, trying pwc driver hack\n", strerror(errno));

		struct v4l2_format fmt;
		CLEAR(fmt);
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(m_Fd, VIDIOC_G_FMT, &fmt)) {
			ERROR("V4l2Capture: VIDIOC_G_FMT failed %s\n", strerror(errno));
			return false;
		}

#define PWC_FPS_SHIFT           16
		fmt.fmt.pix.priv = (fps << PWC_FPS_SHIFT);
		if (-1 == xioctl(m_Fd, VIDIOC_S_FMT, &fmt)) {
			ERROR("V4l2Capture: VIDIOC_S_FMT failed %s\n", strerror(errno));
			return false;
		}
	}*/

	//Truman@100812
	//HRESULT STDMETHODCALLTYPE SetFrameRate(BYTE rate);
	//SetFrameRate(fps);
	return true;
}
int V4l2Capture::SetH264BitRate(unsigned long bitrate )
{
    struct uvc_xu_control_query control_s;
    int err;
    short len = 0;
    char DMA[8];

    LOGD("v4l2 set bitrate: %ul", bitrate);
    control_s.unit = 4;
    control_s.selector = EU_VMH264_CONTROL;
    control_s.size = 8;
    control_s.query = UVC_SET_CUR;
    memset(DMA, 0, 8);
    DMA[0] = 0x1;
    DMA[1] = 0;
    memcpy(&DMA[2], &bitrate, 4);
    DMA[6] = 30;

    control_s.data = (__u8*)DMA;
    if ((err = xioctl (m_Fd, UVCIOC_CTRL_QUERY, &control_s)) < 0) {
        LOGE ("ioctl set control error %d\n", err);
        return -1;
    }
    return err;

}
bool V4l2Capture::SetFormat(uint32_t width, uint32_t height, uint32_t pixFmt) {
	struct v4l2_format fmt;

	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.pixelformat = pixFmt;
	//fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;

	if (-1 == xioctl(m_Fd, VIDIOC_S_FMT, &fmt))
		return false;

	CAP_DBG("width: %d, height: %d, size: %d, bpl: %d\n", fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.sizeimage, fmt.fmt.pix.bytesperline);

	m_FrameSize = fmt.fmt.pix.sizeimage;

	/* Note VIDIOC_S_FMT may change width and height. */
	if (fmt.fmt.pix.width != width || fmt.fmt.pix.height != height) {
		ERROR("VIDIOC_S_FMT changed width or height: wanted %d %d, got %d %d", width,
				height, fmt.fmt.pix.width, fmt.fmt.pix.height);
		return false;
	}

	return true;
}

bool V4l2Capture::ConfigureDev(uint32_t width, uint32_t height, uint32_t pixFmt, uint32_t framerate)
{
	struct v4l2_capability cap;

	if (-1 == xioctl(m_Fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL== errno) {
			LOGD("%s is no V4L2 device\n", m_pDevName);
			FATAL("%s is no V4L2 device\n", m_pDevName);
		} else {
			LOGD("VIDIOC_QUERYCAP failed");
			FATAL("VIDIOC_QUERYCAP failed");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		LOGD("%s is no video capture device\n", m_pDevName);
		FATAL("%s is no video capture device\n", m_pDevName);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		LOGD("%s does not support streaming i/o\n", m_pDevName);
		FATAL("%s does not support streaming i/o\n", m_pDevName);
	}

	if (!SetFormat(width, height, pixFmt)) {
		LOGD("SetFormat failed");
		ERROR("SetFormat failed");
		return false;
	}

	if (!SetFramerate(framerate)) {
		ERROR("SetFramerate failed");
		return false;
	}

	struct v4l2_requestbuffers req;

	CLEAR (req);

	req.count = 12;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (m_Fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			LOGD("%s does not support "
					"memory mapping\n", m_pDevName);
			FATAL("%s does not support "
					"memory mapping\n", m_pDevName);
		} else {
			LOGD("VIDIOC_REQBUFS failed");
			FATAL("VIDIOC_REQBUFS failed");
		}
	}

	CAP_DBG("req.count: %d", req.count);
	if (req.count < 2) {
		LOGD("Insufficient buffer memory on %s\n",
				m_pDevName);
		FATAL("Insufficient buffer memory on %s\n",
				m_pDevName);
	}

	m_pFrameBuf = (FrameBuf *) calloc (req.count, sizeof (*m_pFrameBuf));

	if (!m_pFrameBuf) {
		LOGD("Out of memory\n");
		FATAL("Out of memory\n");
	}

	for (m_NBuffers = 0; m_NBuffers < req.count; ++m_NBuffers) {
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = m_NBuffers;

		if (-1 == xioctl (m_Fd, VIDIOC_QUERYBUF, &buf)) {
			LOGD("VIDIOC_QUERYBUF failed");
			FATAL("VIDIOC_QUERYBUF failed");
        }

		m_pFrameBuf[m_NBuffers].length = buf.length;
		m_pFrameBuf[m_NBuffers].start =
		mmap (NULL /* start anywhere */,
				buf.length,
				PROT_READ | PROT_WRITE /* required */,
				MAP_SHARED /* recommended */,
				m_Fd, buf.m.offset);

		if (MAP_FAILED == m_pFrameBuf[m_NBuffers].start) {
            LOGD("mmap failed");
			FATAL("mmap");
        }
	}

	StartCapturing();
	m_IsConfigured = 1;
	return true;
}

void V4l2Capture::UnConfigureDev()
{
	uint32_t i;

	m_IsConfigured = 0;
	StopCapturing();

	for (i = 0; i < m_NBuffers; ++i)
		if (-1 == munmap(m_pFrameBuf[i].start, m_pFrameBuf[i].length))
			FATAL("munmap");
	free(m_pFrameBuf);
}

void V4l2Capture::StartCapturing()
{
	uint32_t i;
	enum v4l2_buf_type type;

	for (i = 0; i < m_NBuffers; ++i) {
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(m_Fd, VIDIOC_QBUF, &buf))
			FATAL("VIDIOC_QBUF failed");
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl(m_Fd, VIDIOC_STREAMON, &type))
		FATAL("VIDIOC_STREAMON failed");
}

void V4l2Capture::StopCapturing()
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl(m_Fd, VIDIOC_STREAMOFF, &type))
		FATAL("VIDIOC_STREAMOFF failed");
}

void V4l2Capture::ReadFrame(struct v4l2_buffer *buf)
{
	if (!m_IsConfigured) {
		ERROR("Not configured");
		return;
	}
	for (;;) {
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO (&fds);
		FD_SET (m_Fd, &fds);

		/* Timeout. */
		tv.tv_sec = 5;
		tv.tv_usec = 0;
                //LOGD("before sleep\n");
                //sleep(3);
                //LOGD("after sleep\n");
		r = select(m_Fd + 1, &fds, NULL, NULL, &tv);
                //LOGD("after select, r = %d\n", r);
		if (-1 == r) {
			if (EINTR== errno)
				continue;
            LOGE("read frame, select failed");
			FATAL("select failed");
		}

		if (0 == r) {
            LOGE("read frame, select timeout\n");
			FATAL("read frame, select timeout\n");
		}

		CLEAR (*buf);

		buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf->memory = V4L2_MEMORY_MMAP;

		r = xioctl (m_Fd, VIDIOC_DQBUF, buf);
        //printf("after VIDIOC_DQBUF, r = %d, errno = %d\n", r, errno);
		if (-1 == r) {
			switch (errno) {
			case EAGAIN:
				continue;
			case EIO:
                FATAL("VIDIOC_DQBUF returned EIO");
				WARN("VIDIOC_DQBUF returned EIO");
				return;
			default:
                LOGE("VIDIOC_DQBUF failed");
				FATAL("VIDIOC_DQBUF failed");
			}
		} else {
			assert (buf->index < m_NBuffers);
			return;
		}
	}
}

void V4l2Capture::CaptureFrame(uint8_t *frameBuf, uint32_t *frameBufLen)
{
	struct v4l2_buffer buf;
    
	ReadFrame(&buf);
    LOGD("cp buf size:%d, frameBufLen: %p", buf.bytesused, frameBufLen);
	memcpy(frameBuf, m_pFrameBuf[buf.index].start, buf.bytesused);

	if (-1 == xioctl(m_Fd, VIDIOC_QBUF, &buf))
		FATAL("VIDIOC_QBUF failed");
    
    *frameBufLen = buf.bytesused;
}

int V4l2Capture::CaptureFrame(MediaSample &sample)
{
	struct v4l2_buffer buf;

	ReadFrame(&buf);
    //CAP_DBG("V4l2Capture:  Got a frame from device. \r\n");
	//assert(m_FrameSize == frameBufLen);

	//memcpy(frameBuf, m_pFrameBuf[buf.index].start, m_FrameSize);
	//if(sample.GetBufferSize() >= m_FrameSize)
	if(sample.GetBufferSize() >= buf.bytesused)
	{
        memcpy(sample.GetBuffer(),m_pFrameBuf[buf.index].start,buf.bytesused);
        //sample.SetSampleSize(m_FrameSize);
        sample.SetSampleSize(buf.bytesused);
        //CAP_DBG("Frame size = 0x%X \r\n",sample.GetSampleSize());
        if (-1 == xioctl(m_Fd, VIDIOC_QBUF, &buf))
            FATAL("VIDIOC_QBUF failed");
        m_FrameSize = buf.bytesused;
        return m_FrameSize;
	}
	else
	{
        if (-1 == xioctl(m_Fd, VIDIOC_QBUF, &buf))
            FATAL("VIDIOC_QBUF failed");
            return 0;
	}
}
