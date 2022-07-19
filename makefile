#-----------------------------------------------------------------------------
# These are the variables that are specific to your program
#-----------------------------------------------------------------------------
EXE = pcitool
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Find out what kind of hardware we're compiling on
#-----------------------------------------------------------------------------
PLATFORM := $(shell uname)

#-----------------------------------------------------------------------------
# For x86, declare whether to emit 32-bit or 64-bit code
#-----------------------------------------------------------------------------
X86_TYPE = 64

#-----------------------------------------------------------------------------
# These are the language standards we want to compile with
#-----------------------------------------------------------------------------
C_STD = c99
CPP_STD = c++17

#-----------------------------------------------------------------------------
# Declare where the object files get created
#-----------------------------------------------------------------------------
X86_OBJ_DIR = obj_x86

#-----------------------------------------------------------------------------
# Tell 'make' what suffixes will appear in make rules
#-----------------------------------------------------------------------------
.SUFFIXES:
.SUFFIXES: .o .c .cpp

#-----------------------------------------------------------------------------
# Declare the compile-time flags that are common between all platforms
#-----------------------------------------------------------------------------
CXXFLAGS =	\
-O2 -g -Wall \
-Wno-write-strings \
-Wno-sign-compare \
-Wno-unused-result \
-Wno-strict-aliasing \
-fcommon \
-DLINUX 

#-----------------------------------------------------------------------------
# If there was no goal on the command line, the default goal depends
# on what platform we're running on
#-----------------------------------------------------------------------------
ifeq ($(.DEFAULT_GOAL),)
    ifeq ($(PLATFORM), Linux)
        .DEFAULT_GOAL := x86
    endif
endif


#-----------------------------------------------------------------------------
# Define the name of the compiler and what "build all" means for our platform
#-----------------------------------------------------------------------------
ifeq ($(PLATFORM), Linux)
    ALL       = x86 
    X86_CC    = $(CC)
    X86_CXX   = $(CXX)
    X86_STRIP = strip
endif


#-----------------------------------------------------------------------------
# We're going to compile every .c and .cpp file in this folder
#-----------------------------------------------------------------------------
C_SRC_FILES := $(wildcard *.c)
CPP_SRC_FILES := $(wildcard *.cpp)


#-----------------------------------------------------------------------------
# Create the base-names of the object files
#-----------------------------------------------------------------------------
C_OBJ   = $(C_SRC_FILES:.c=.o)
CPP_OBJ = $(CPP_SRC_FILES:.cpp=.o)
OBJ_FILES = ${C_OBJ} ${CPP_OBJ}


#-----------------------------------------------------------------------------
# We are going to keep x86 and ARM object files in separate sub-directories
#-----------------------------------------------------------------------------
X86_OBJS = $(addprefix $(X86_OBJ_DIR)/,$(OBJ_FILES))
ARM_OBJS = $(addprefix $(ARM_OBJ_DIR)/,$(OBJ_FILES))


#-----------------------------------------------------------------------------
# This rules tells how to compile an X86 .o object file from a .cpp source
#-----------------------------------------------------------------------------
$(X86_OBJ_DIR)/%.o : %.cpp
	$(X86_CXX) -m$(X86_TYPE) $(CPPFLAGS) -std=$(CPP_STD) $(CXXFLAGS) -c $< -o $@

$(X86_OBJ_DIR)/%.o : %.c
	$(X86_CC) -m$(X86_TYPE) $(CPPFLAGS) -std=$(C_STD) $(CXXFLAGS) -c $< -o $@

#-----------------------------------------------------------------------------
# This rules tells how to compile an ARM .o object file from a .cpp source
#-----------------------------------------------------------------------------
$(ARM_OBJ_DIR)/%.o : %.cpp
	$(ARM_CXX) $(CPPFLAGS) -std=$(CPP_STD) $(CXXFLAGS) $(ARMFLAGS) -c $< -o $@

$(ARM_OBJ_DIR)/%.o : %.c
	$(ARM_CC) $(CPPFLAGS) -std=$(C_STD) $(CXXFLAGS) $(ARMFLAGS) -c $< -o $@

#-----------------------------------------------------------------------------
# This rule builds the x86 executable from the object files
#-----------------------------------------------------------------------------
$(EXE) : $(X86_OBJS)
	$(X86_CXX) -m$(X86_TYPE) -pthread -o $@ $(X86_OBJS)
	$(X86_STRIP) $(EXE)


#-----------------------------------------------------------------------------
# This target builds all executables supported by this platform
#-----------------------------------------------------------------------------
all:	$(ALL)


#-----------------------------------------------------------------------------
# This target builds just the x86 executable
#-----------------------------------------------------------------------------
x86:	mkdirs $(EXE)

#-----------------------------------------------------------------------------
# This target configures the object file directories
#-----------------------------------------------------------------------------
$(X86_OBJ_DIR):
	@mkdir $(X86_OBJ_DIR)


#-----------------------------------------------------------------------------
# This target configured the object file directories
#-----------------------------------------------------------------------------
mkdirs:	$(X86_OBJ_DIR)
	@chmod 777 $(X86_OBJ_DIR)

#-----------------------------------------------------------------------------
# This target removes all files that are created at build time
#-----------------------------------------------------------------------------
clean:
	rm -rf Makefile.bak makefile.bak $(EXE).tgz $(EXE)
	rm -rf $(X86_OBJ_DIR) $(ARM_OBJ_DIR)

#-----------------------------------------------------------------------------
# This target creates a compressed tarball of the source code
#-----------------------------------------------------------------------------
tarball:	clean
	rm -rf $(EXE).tgz
	tar --create --exclude-vcs -v -z -f $(EXE).tgz *

    
#-----------------------------------------------------------------------------
# This target appends/updates the dependencies list at the end of this file
#-----------------------------------------------------------------------------
depend:	x86
	makedepend -p$(X86_OBJ_DIR)/ -Y 2>/dev/null


#-----------------------------------------------------------------------------
# Convenience target for displaying makefile variables 
#-----------------------------------------------------------------------------
debug:
	@echo C_OBJ = ${C_OBJ}
	@echo CPP_OBJ = ${CPP_OBJ}
	@echo OBJ_FILES = ${OBJ_FILES}
# DO NOT DELETE
