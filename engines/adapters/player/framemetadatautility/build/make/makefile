# Include the project-wide makefile to get 
# appropriate definitions
include $(PV_TOP)/makefile.pv

WHAT = library
TARGET = pvframemetadatautility

# Use XCFLAGS to add extra compiler directives for the C compiler
# if you add any please use "XCFLAGS += ..." not "XCFLAGS = " 
# unless you're sure you want to override the project defaults.
# Please include a comment if you do this.
#
# XCFLAGS += 

# Use XCXXFLAGS to add extra compiler directives for the C++ compiler.
# See notes above regarding overriding project defaults.
#
# XCXXFLAGS += 

# Use XCPPFLAGS to add extra preprocessor directives. Use XINCDIRS
# for other include directories, but please only do this if absolutely
# necessary -- let the makefile system find the include directories if 
# possible.
# See notes above regarding overriding project defaults.
#
#XCPPFLAGS += 

# Use XINCDIRS for extra include directories. These should be proceeded 
# with "-I" just as they would be when passing to the compiler.
#
# Added this to for osclconfig.h
#XINCDIRS += -I../../../config/$(BUILD_ARCH)  -I../../../config/shared

XINCDIRS += -I $(VOB_BASE_DIR)/engines/common/include \
            -I $(VOB_BASE_DIR)/engines/player/include \
            -I ../../config/linux \
			-I $(VOB_BASE_DIR)/nodes/common/include \
			-I $(VOB_BASE_DIR)/pvmi/pvmf/include \
			-I $(VOB_BASE_DIR)/baselibs/pv_mime_utils/src \
			-I $(VOB_BASE_DIR)/nodes/pvmediaoutputnode/include \
			-I $(VOB_BASE_DIR)/codecs_v2/utilities/colorconvert/include \
			-I $(VOB_BASE_DIR)/codecs_v2/utilities/colorconvert/src

# Use XLIBDIRS for extra library directories. These should be proceeded 
# with "-L" just as they would be when passing to the linker. 
# Used for building executables.
#
# XLIBDIRS += 

# Use XLDFLAGS for extra linker directives.
# Used for building executables.
#
# XLDFLAGS += 

SRCDIR = ../../src
INCSRCDIR = ../../include

# compose final src list for actual build
SRCS =	pv_frame_metadata_utility.cpp \
	pv_frame_metadata_factory.cpp \
	pv_frame_metadata_mio_video.cpp \
	pv_frame_metadata_mio_audio.cpp \
	../config/common/pv_frame_metadata_mio_video_config.cpp

HDRS =  pv_frame_metadata_factory.h \
	pv_frame_metadata_interface.h

DESTDIR = $(SDK_LOCAL)/installed_lib/$(BUILD_ARCH)
INCDESTDIR = $(SDK_LOCAL)/installed_include
include $(MK)/gnumake.template



