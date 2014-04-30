KERNEL_DIR	?= /home/android/out/obj/KERNEL_OBJ
PWD		:= $(shell pwd)
obj-m		:= v4l2loopback.o
CCPATH := ${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64/bin

MODULE_OPTIONS = devices=2

##########################################
# note on build targets
#
# module-assistant makes some assumptions about targets, namely
#  <modulename>: must be present and build the module <modulename>
#                <modulename>.ko is not enough
# install: must be present (and should only install the module)
#
# we therefore make <modulename> a .PHONY alias to <modulename>.ko
# and remove utils-installation from 'install'
# call 'make install-all' if you want to install everything
##########################################


.PHONY: all clean

# we don't control the .ko file dependencies, as it is done by kernel
# makefiles. therefore v4l2loopback.ko is a phony target actually
.PHONY: v4l2loopback.ko

all: v4l2loopback.ko
v4l2loopback: v4l2loopback.ko
v4l2loopback.ko:
	@echo "Building v4l2-loopback driver..."
	$(MAKE) ARCH=arm CROSS_COMPILE=$(CCPATH)/arm-linux-androideabi- -C $(KERNEL_DIR) M=$(PWD) modules


clean:
	rm -f *~
	rm -f Module.symvers Module.markers modules.order
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
