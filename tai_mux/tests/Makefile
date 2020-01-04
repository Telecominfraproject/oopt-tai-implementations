ifndef TAI_DIR
    TAI_DIR := ../oopt-tai
endif

ifndef TAI_LIB_DIR
    TAI_LIB_DIR := $(TAI_DIR)/tools/framework
endif

all: ../libtai.so static.json $(TAI_LIB_DIR)/examples/basic/libtai.so
	TAI_MUX_STATIC_CONFIG_FILE=$(abspath static.json) TAI_TEST_TARGET=$(abspath ../libtai.so) $(MAKE) -C $(TAI_DIR)/tests

../libtai.so:
	$(MAKE) -C $(@D)

$(TAI_LIB_DIR)/examples/basic/libtai.so:
	$(MAKE) -C $(@D)
