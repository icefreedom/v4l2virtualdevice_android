#ifndef MEDIASAMPLE_H
#define MEDIASAMPLE_H

#include <stdint.h>
class MediaSample
{
     public:
        MediaSample()
        :is_ext_buf(0)
        ,buf(0)
        ,buf_used(0)
        ,buf_size(0)
        {}
        virtual ~MediaSample()
        {
            if( is_ext_buf==false && buf!=0)
                delete buf;
        }
        bool AllocBuffer(uint32_t size,uint8_t* ext_buf=0)
        {
            if( is_ext_buf==false && buf!=0)
            {
                delete buf;
                buf = 0;
            }

            if(ext_buf)
            {
                is_ext_buf = true;
                buf_size = size;
                buf_used = 0;
                buf = ext_buf;
            }
            else
            {
                is_ext_buf = false;
                buf_size = size;
                buf_used = 0;
                buf = new uint8_t[size];
            }
            return true;
        }

        bool SetSampleSize(uint32_t size)
        {
            if(buf)
            {
                buf_used = size;
                return true;
            }
            else
                return false;
        }

        uint32_t GetSampleSize()
        {
            return buf_used;
        }

        uint32_t GetBufferSize()
        {
            return buf_size;
        }

        uint8_t* GetBuffer()
        {
            return buf;
        }
    private:
        bool is_ext_buf;
        uint8_t *buf;
        uint32_t buf_used;
        uint32_t buf_size;
};

#endif
