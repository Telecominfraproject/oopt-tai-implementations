TAI_DIR ?= oopt-tai
TAI_FRAMEWORK_PLATFORM_HEADER ?= mux.hpp
TAI_PROG ?= libtai-mux.so
TAI_META_CUSTOM_FILES ?= $(abspath $(wildcard custom_attrs/*))
VENDOR_LDFLAGS := -ldl

TAI_DOCKER_CMD ?= bash
TAI_DOCKER_RUN_OPTION ?= -it --rm

.PHONY = all test cmd docker builder bash clean-all

include $(TAI_DIR)/tools/framework/make/lib.mk

test:
	$(MAKE) -C tests

cmd:
	TAI_DOCKER_MOUNT="`pwd`:/data" $(MAKE) -C $(TAI_DIR) $@

docker:
	TAI_DOCKER_CMD="make" $(MAKE) cmd

builder:
	$(MAKE) -C $(TAI_DIR) $@

bash:
	$(MAKE) cmd

ci: clean-all builder
	TAI_DOCKER_RUN_OPTION='-t --rm' TAI_DOCKER_CMD='make test' $(MAKE) cmd

clean-all: clean
	git clean -dfx oopt-tai
