CC := g++
CFLAGS := -std=c++11
INCLUDES := -I ../oopt-tai/inc

test: test.cpp libtai.so libtai-stub.so libmetatai.so
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ -L . -ltai -L ../oopt-tai/meta -lmetatai

libtai.so:
	ln -s ../libtai-mux.so $@

libmetatai.so:
	$(MAKE) -C ../oopt-tai/meta/
	ln -s ../oopt-tai/meta/$@ $@

libtai-stub.so:
	$(MAKE) -C ../oopt-tai/stub/
	ln -s ../oopt-tai/stub/libtai.so $@

clean:
	-rm test
