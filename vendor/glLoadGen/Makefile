CC = gcc

BUILDDIR = lib
SRCDIR = src

INCLUDES = -Iinclude/GL/

CFLAGS = -O3 -lto -Wall -Winline -Wextra -Wfatal-errors -std=c11 -m64 -march=native $(INCLUDES)

SOURCES = $(shell find $(SRCDIR) -type f -name "*.c")
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/lib%.a,$(SOURCES))

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR)/%.d
	@mkdir -p $(shell dirname $@)
	@echo " CC $<"; $(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.d: $(SRCDIR)/%.c
	@mkdir -p $(shell dirname $@)
	@echo " DEP $@"; $(CC) $(CFLAGS) -MM -MT"$(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$<)" $< -MF"$@"

$(BUILDDIR)/lib%.a: $(BUILDDIR)/%.o
	@echo " AR $@"; ar cr $@ $<

gen:
	@lua LoadGen.lua core_4_4 -style=pointer_c -spec=gl -version=4.4 -profile=core -stdext=g l_ubiquitous.txt -stdext=gl_core_post_3_3.txt -stdext=gl_plat_3_3.txt

all: $(OBJECTS)