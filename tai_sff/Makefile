PROG := libtai.so

ifndef TAI_DIR
    TAI_DIR := oopt-tai
endif

ifndef TAI_LIB_DIR
    TAI_LIB_DIR := $(TAI_DIR)/tools/lib
endif

ifndef TAI_FRAMEWORK_DIR
    TAI_FRAMEWORK_DIR := $(TAI_DIR)/tools/framework
endif

ifndef TAI_META_LIBRARY
    TAI_META_LIBRARY := $(TAI_DIR)/meta/libmetatai.so
endif

ifndef TAI_DOCKER_CMD
    TAI_DOCKER_CMD := bash
endif

ifndef TAI_SFF_BUILDDIR
    TAI_SFF_BUILDDIR := build
endif

CFLAGS := -std=c++17 -g3 -shared -fPIC -DTAI_EXPOSE_PLATFORM -fno-gnu-unique

INCLUDES := -I $(TAI_DIR)/inc -I $(TAI_DIR)/meta -I $(TAI_LIB_DIR) -I $(TAI_FRAMEWORK_DIR) -include sff.hpp

BUILDDIR := $(TAI_SFF_BUILDDIR)
SOURCES := $(wildcard *.cpp $(TAI_LIB_DIR)/*.cpp $(TAI_FRAMEWORK_DIR)/*.cpp)
HEADERS := $(wildcard *.hpp $(TAI_LIB_DIR)/*.hpp $(TAI_FRAMEWORK_DIR)/*.hpp)
OBJECTS := $(addprefix $(BUILDDIR)/,$(SOURCES:%.cpp=%.o))
DEPS := $(addprefix $(BUILDDIR)/,$(SOURCES:%.cpp=%.d))

.PHONY = all meta test cmd docker docker-image bash clean clean-all

all: $(PROG)

$(PROG): $(TAI_META_LIBRARY) $(OBJECTS) $(HEADERS)
	$(CXX) $(CFLAGS) $(INCLUDES) -shared $(OBJECTS) -o $@ -ldl -lpthread -L $(dir $(TAI_META_LIBRARY)) -lmetatai

$(TAI_META_LIBRARY): $(wildcard custom_attrs/*)
	TAI_META_CUSTOM_FILES="$(abspath $(wildcard custom_attrs/*))" $(MAKE) -C $(@D)

meta: $(TAI_META_LIBRARY)

$(BUILDDIR)/%.o: %.cpp Makefile
	mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -MMD -MP -MF $(BUILDDIR)/$*.d -o $@

-include $(DEPS)

test:
	$(MAKE) -C tests

cmd:
	TAI_DOCKER_CMD="$(TAI_DOCKER_CMD)" TAI_DOCKER_RUN_OPTION="--privileged --net=host -it --rm" TAI_DOCKER_MOUNT="`pwd`:/data" $(MAKE) -C $(TAI_DIR) $@

docker:
	TAI_DOCKER_CMD="make" $(MAKE) cmd

docker-image:
	$(MAKE) -C $(TAI_DIR) $@

bash:
	$(MAKE) cmd

clean:
	$(RM) -r $(BUILDDIR) $(TARGET)

clean-all: clean
	$(MAKE) -C $(TAI_DIR) clean
