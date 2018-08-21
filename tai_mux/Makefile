CC := g++
CFLAGS := -std=c++11 -fPIC
INCLUDES := -I ./oopt-tai/inc
BUILDDIR := build
SRCDIR := .
SRCEXT := cpp
SOURCES := $(shell ls $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

TARGET := libtai-mux.so

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -shared $^ -o $@ -ldl -lpthread

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	-rm -r $(BUILDDIR) $(TARGET)
