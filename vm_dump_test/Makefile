ifndef verbose
  SILENT = @
endif
VERSION = 0.1

#SYSROOT=/home/hf/android-ndk-r8/platforms/android-14/arch-arm
#CC = /home/hf/android-ndk-r8/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-gcc --sysroot=$(SYSROOT)
#CXX = /home/hf/android-ndk-r8/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-g++ --sysroot=$(SYSROOT)
#AR = /home/hf/android-ndk-r8/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-ar --sysroot=$(SYSROOT)
SYSROOT=$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm
CC = $(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-gcc --sysroot=$(SYSROOT)
CXX = $(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-g++ --sysroot=$(SYSROOT)
AR = $(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-ar

#CC = ../toolchain/crosstools/bin/mipsel-linux-gcc
#CXX = ../toolchain/crosstools/bin/mipsel-linux-g++
#AR = ../toolchain/crosstools/bin/mipsel-linux-ar

#CC = gcc
#CXX = g++
#AR = ar

#CC = /home/andyhsieh/prj/Freescale/arm-eabi-4.4.0/bin/gcc/arm-eabi-gcc
#CXX = /home/andyhsieh/prj/Freescale/arm-eabi-4.4.0/bin/arm-eabi-g++
#AR = /home/andyhsieh/prj/Freescale/arm-eabi-4.4.0/bin/arm-eabi-ar

#CC = arm-fsl-linux-gnueabi-gcc
#CXX = arm-fsl-linux-gnueabi-g++
#AR = arm-fsl-linux-gnueabi-ar
#LIBS = -laituvc_fsl

ifndef CC
  CC = gcc
endif

ifndef CXX
  CXX = g++
endif

ifndef AR
  AR = ar
endif

TARGETDIR  =
TARGET     = $(TARGETDIR)bin/vmc_test
DEFINES    += -D_DEBUG -DDEBUG -DENABLE_DEBUG
CPPFLAGS   += -MMD -MP $(DEFINES) $(INCLUDES) -DLINUX -DVERSION=\"$(VERSION)\"
CFLAGS     += $(CPPFLAGS) $(ARCH) -Wall -g
CXXFLAGS   += $(CFLAGS) -fno-rtti -fno-exceptions -g
#LDFLAGS    += -llog
LIBS      += 
RESFLAGS  += $(DEFINES) $(INCLUDES)
LDDEPS    +=
SRC	   = V4l2Capture.cpp main.cpp
OBJECTS	   = V4l2Capture.o main.o

all:
	$(CXX) -c $(SRC) $(INCLUDES) $(CXXFLAGS)
	$(CXX) -o $(TARGET) $(OBJECTS) -L./ $(LIBS) $(LDFLAGS)
	$(AR) -rc libzxwcapture.a $(OBJECTS) 


clean:
	rm -f $(OBJECTS) $(TARGET)

