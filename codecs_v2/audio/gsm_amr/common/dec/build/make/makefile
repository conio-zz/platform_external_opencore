#Include the project-wide makefile to get
# appropriate definitions

include $(PV_TOP)/makefile.pv


# BE CAREFUL NOT TO HAVE ANY TRAILING WHITESPACE FOR THE NEXT 3 LINES BELOW OR IT WILL NOT WORK!
WHAT = library
TARGET =
# Use XCFLAGS to add extra compiler directives for the C compiler
# if you add any please use "XCFLAGS += ..." not "XCFLAGS = "
# unless you're sure you want to override the project defaults.
# Please include a comment if you do this.
#
# XCFLAGS = 

# Use XCXXFLAGS to add extra compiler directives for the C++ compiler.
# See notes above regarding overriding project defaults.
#
# XCXXFLAGS += -Wall -g -DUSE_UNICODE -D_POSIX_PTHREAD_SEMANTICS
XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

# Use XCPPFLAGS to add extra preprocessor directives. Use XINCDIRS
# for other include directories, but please only do this if absolutely
# necessary -- let the makefile system find the include directories if
# possible.
# See notes above regarding overriding project defaults.
#
#XCPPFLAGS += -DORIGINAL_TYPEDEF_H -DOSCL_BASE_H_INCLUDED -DOSCL_TYPES_H_INCLUDE -DOSCL_ASSERT_H_INCLUDED -DOSCL_MEM_BASIC_FUNCTIONS_H -DOSCL_MEM_IMP_H
XCPPFLAGS += 

# Use XINCDIRS for extra include directories. These should be proceeded
# with "-I" just as they would be when passing to the compiler.
#
# XINCDIRS +=

# Use XLIBDIRS for extra library directories. These should be proceeded
# with "-L" just as they would be when passing to the linker.
# Used for building executables.
#
XLIBDIRS += 

# Use XLDFLAGS for extra linker directives.
# Used for building executables.
#
# XLDFLAGS +=

INCSRCDIR = ../../include

HDRS = pvgsmamrdecoderinterface.h

LIBS =

DESTDIR = $(SDK_LOCAL)/installed_lib/$(BUILD_ARCH)
INCDESTDIR = $(SDK_LOCAL)/installed_include

include $(MK)/gnumake.template

