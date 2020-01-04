PROG := libtai.so

ifndef TAI_DIR
    TAI_DIR := oopt-tai
endif

ifndef TAI_LIB_DIR
    TAI_LIB_DIR := $(TAI_DIR)/tools/framework
endif

ifndef TAI_META_LIBRARY
    TAI_META_LIBRARY := $(TAI_DIR)/meta/libmetatai.so
endif

CFLAGS := -std=c++17 -g3 -shared -fPIC -DTAI_EXPOSE_PLATFORM -fno-gnu-unique

INCLUDES := -I $(TAI_DIR)/inc -I $(TAI_DIR)/meta -I $(TAI_LIB_DIR) -include mux.hpp

BUILDDIR := build
SOURCES := $(wildcard *.cpp) $(wildcard $(TAI_LIB_DIR)/*.cpp)
HEADERS := $(wildcard *.hpp) $(wildcard $(TAI_LIB_DIR)/*.hpp)
OBJECTS := $(addprefix build/,$(SOURCES:%.cpp=%.o))
DEPS := $(addprefix build/,$(SOURCES:%.cpp=%.d))

all: $(PROG)

$(PROG): $(TAI_META_LIBRARY) $(OBJECTS) $(HEADERS)
	$(CXX) $(CFLAGS) $(INCLUDES) -shared $(OBJECTS) -o $@ -ldl -lpthread -L $(dir $(TAI_META_LIBRARY)) -lmetatai

$(TAI_META_LIBRARY):
	$(MAKE) -C $(@D)

$(BUILDDIR)/%.o: %.cpp Makefile
	mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -MMD -MP -MF build/$*.d -o $@

-include $(DEPS)

clean:
	$(RM) -r $(BUILDDIR) $(TARGET)

bash:
	TAI_DOCKER_RUN_OPTION="--privileged --net=host -it --rm" TAI_DOCKER_MOUNT="`pwd`:/data" $(MAKE) -C $(TAI_DIR) $@
