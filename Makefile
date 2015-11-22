CC := $(CXX)
RM ?= rm -f

SRCDIR = src
VNDDIR = vendor
BUILDDIR = tmp
ARDIR = lib
ARNAME = libcore
PROGRAM = main
CXXFLAGS = -Wall -Winline -Wextra -Wfatal-errors -std=c++14 -m64 -march=native
CXXFLAGS_DEBUG = -g -Og -Weffc++ -Winvalid-pch
CXXFLAGS_RELEASE = -O3 -flto -funroll-loops -msse -msse2 -msse3 -mfpmath=sse
LFLAGS = -static
LFLAGS_DEBUG =
LFLAGS_RELEASE = -flto -s
LDIR =
LIBS = -lgcc -lglfw3 -lgl_core_4_4 -lopengl32 -lstb_image -lfreetype -lminball

DEFINES = \
	-DNGN_USE_GLLOADGEN

ifeq ($(MAKECMDGOALS),release)
	CXXFLAGS += $(CXXFLAGS_RELEASE)

	LFLAGS += $(LFLAGS_RELEASE)
else
	CXXFLAGS += $(CXXFLAGS_DEBUG)
	DEFINES += \
		-DDEBUG \
		-D_DEBUG

	LFLAGS += $(LFLAGS_DEBUG)
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

$(SRCDIR)/pch.hpp.gch: $(SRCDIR)/pch.hpp
	@mkdir -p $(shell dirname $@)
	@echo " PCH $<"; $(CC) $(CXXFLAGS) -x c++-header -c $< -o $@

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

pch: $(SRCDIR)/pch.hpp.gch

run: all
	./$(PROGRAM) ${ARGS}

run-gdb: all
	gdb $(PROGRAM) ${ARGS}

clean-pch:
	@echo "Cleaning pch file..."
	@$(RM) -v $(SRCDIR)/pch.hpp.gch

clean-dep:
	@echo "Cleaning dep files..."
	@find $(BUILDDIR) -name '*.d' -type f | xargs $(RM) -v

clean-obj:
	@echo "Cleaning obj files..."
	@$(RM) -r ./$(PROGRAM)
	@find $(BUILDDIR) -name '*.o' -type f | xargs $(RM) -v

clean: clean-dep clean-obj clean-pch

ar:
	@echo "Creating archive..."
	@mkdir -p $(ARDIR)
	@ar cr $(ARDIR)/$(ARNAME).a `find $(BUILDDIR) -name '*.o' -type f`
	@echo "  $(ARDIR)/$(ARNAME).a  done"

.PHONY: all release force pch run run-gdb clean clean-dep clean-obj archive