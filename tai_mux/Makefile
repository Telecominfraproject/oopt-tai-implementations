CC := g++
CFLAGS := -std=c++11 -fPIC
INCLUDES := -I ./oopt-tai/inc -I ./oopt-tai/meta/
BUILDDIR := build
SRCDIR := .
SRCEXT := cpp
SOURCES := $(shell ls $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

TARGET := libtai-mux.so

all: $(TARGET)

oopt-tai/meta/taimetadata.h:
	$(MAKE) -C ./oopt-tai/meta

$(TARGET): oopt-tai/meta/taimetadata.h $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -shared $^ -o $@ -ldl -lpthread -L ./oopt-tai/meta -lmetatai

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	-rm -r $(BUILDDIR) $(TARGET)
