ifndef TAI_DIR
    TAI_DIR := ../oopt-tai
endif

ifndef TAI_LIB_DIR
    TAI_LIB_DIR := $(TAI_DIR)/tools/framework
endif

all: exec-pa
	TAI_MUX_STATIC_CONFIG_FILE=$(abspath static.json) TAI_TEST_TARGET=$(abspath libtai.so) $(MAKE) -C $(TAI_DIR)/tests

exec-pa: libtai.so static.json libtai-a.so libtai-b.so
	TAI_MUX_PLATFORM_ADAPTER="exec" TAI_MUX_EXEC_SCRIPT=$(abspath exec.py) LD_LIBRARY_PATH=. python -m unittest -vf

libtai.so:
	$(MAKE) -C ..
	ln -sf ../$@ $@

libtai-a.so:
	TAI_META_CUSTOM_FILES="$(abspath $(wildcard custom_a/*.h))" $(MAKE) -C $(TAI_DIR)/meta clean
	TAI_META_CUSTOM_FILES="$(abspath $(wildcard custom_a/*.h))" $(MAKE) -C $(TAI_LIB_DIR)/examples/basic clean
	TAI_META_CUSTOM_FILES="$(abspath $(wildcard custom_a/*.h))" $(MAKE) -C $(TAI_LIB_DIR)/examples/basic
	cp $(TAI_LIB_DIR)/examples/basic/libtai.so $@

libtai-b.so:
	TAI_META_CUSTOM_FILES="$(abspath $(wildcard custom_b/*.h))" $(MAKE) -C $(TAI_DIR)/meta clean
	TAI_META_CUSTOM_FILES="$(abspath $(wildcard custom_b/*.h))" $(MAKE) -C $(TAI_LIB_DIR)/examples/basic clean
	TAI_META_CUSTOM_FILES="$(abspath $(wildcard custom_b/*.h))" $(MAKE) -C $(TAI_LIB_DIR)/examples/basic
	cp $(TAI_LIB_DIR)/examples/basic/libtai.so $@

run: libtai-a.so libtai-b.so taish
	TAI_MUX_STATIC_CONFIG_FILE=static.json LD_LIBRARY_PATH=..:$(abspath .) $(TAI_DIR)/tools/taish/taish_server -vn

taish:
	$(MAKE) -C $(TAI_DIR)/tools/taish

clean:
	$(RM) libtai-a.so libtai-b.so
	$(MAKE) -C $(TAI_DIR)/tools/taish clean
