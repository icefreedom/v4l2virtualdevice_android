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

#ifndef V4L2CAPTURE_HPP_
#define V4L2CAPTURE_HPP_

#include <stdint.h>
#include "MediaSample.hpp"

struct FrameBuf;

class V4l2Capture
{
public:
	V4l2Capture(const char *devName);
	~V4l2Capture();
	bool ConfigureDev(uint32_t width, uint32_t height, uint32_t pixFmt, uint32_t framerate);
	void UnConfigureDev();
	void CaptureFrame(uint8_t *frameBuf, uint32_t *frameBufLen);
	int CaptureFrame(MediaSample &sample);
	int GetDevHandle(){return m_Fd;}
	int m_Fd;
    int SetH264BitRate(unsigned long bitrate );

protected:
    FrameBuf *m_pFrameBuf;

private:
	void OpenDevice();
	void CloseDevice();
	bool SetFormat(uint32_t width, uint32_t height, uint32_t pixFmt);
	void ReadFrame(struct v4l2_buffer *buf);
	void StartCapturing();
	void StopCapturing();
	bool SetFramerate(int fps);
    const char *m_pDevName;
    int m_IsConfigured;
    uint32_t m_FrameSize;
    uint32_t m_NBuffers;
};


#endif /* V4L2CAPTURE_HPP_ */
