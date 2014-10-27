CC := $(CXX)
RM ?= rm -f

SRCDIR = src
VNDDIR = vendor
BUILDDIR = tmp
ARDIR = lib
ARNAME = libcore
PROGRAM = main
CXXFLAGS = -Wall -Winline -Wextra -std=c++11 -m64 -march=native
CXXFLAGS_DEBUG = -g -Og -Weffc++ -Wfatal-errors -DDEBUG
CXXFLAGS_RELEASE = -O3 -Wfatal-errors -funroll-loops -msse -msse2 -msse3 -mfpmath=sse
LFLAGS =
LDIR =
LIBS = -static -lgcc -lglfw3 -lgl3w -lopengl32 -lstb_image -lfreetype

DEFINES = \
	-DGLM_FORCE_RADIANS \
	-DGLM_FORCE_CXX11 \

ifeq ($(MAKECMDGOALS),release)
	CXXFLAGS += $(CXXFLAGS_RELEASE) \

else
	CXXFLAGS += $(CXXFLAGS_DEBUG)
	DEFINES += \
		-DDEBUG \
		-D_DEBUG \

endif

VENDORS := $(shell find $(VNDDIR) -maxdepth 3 -type d -name include)

ifneq (,$(findstring Windows,$(OS)))
	include Makefile.windows
endif

CXXFLAGS += $(DEFINES) $(INCLUDES) -I$(SRCDIR)
LFLAGS += $(LDIR)

SOURCES  := $(shell find $(SRCDIR) -type f -name "*.cpp")
OBJECTS  := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))
DEPFILES := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.d,$(SOURCES))

ifneq (,$(findstring clean,$(MAKECMDGOALS)))
	DEPFILES =
endif

$(PROGRAM): $(OBJECTS)
	@echo " Linking..."; $(CC) $(LFLAGS) $^ $(LIBS) -o $(PROGRAM)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(BUILDDIR)/%.d
	@mkdir -p $(shell dirname $@)
	@echo " CC $<"; $(CC) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: $(SRCDIR)/%.cpp
	@mkdir -p $(shell dirname $@)
	@echo " DEP $@"; $(CC) $(CXXFLAGS) -MM -MT"$(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$<)" $< -MF"$@"

-include $(DEPFILES)

# asm: src/main.cpp
# 	@echo " ASM main.cpp"; $(CC) $(CXXFLAGS) -S -fverbose-asm src/main.cpp $(LIBS) -o main.asm; cat main.asm | c++filt > main.s; rm main.asm

all: $(PROGRAM)

release: clean all
	@echo "Release the Cracken!"

force: clean all

run: all
	./$(PROGRAM) ${ARGS}

run-gdb: all
	gdb $(PROGRAM) ${ARGS}

clean-dep:
	@echo "Cleaning dep files..."
	@find $(BUILDDIR) -name '*.d' -type f | xargs $(RM) -v

clean-obj:
	@echo "Cleaning obj files..."
	@$(RM) -r ./$(PROGRAM)
	@find $(BUILDDIR) -name '*.o' -type f | xargs $(RM) -v

clean: clean-dep clean-obj

ar:
	@echo "Creating archive..."
	@mkdir -p $(ARDIR)
	@ar cr $(ARDIR)/$(ARNAME).a `find $(BUILDDIR) -name '*.o' -type f`
	@echo "  $(ARDIR)/$(ARNAME).a  done"

.PHONY: all release force run run-gdb clean clean-dep clean-obj archive