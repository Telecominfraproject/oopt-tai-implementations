CC := g++
CFLAGS := -std=c++11
INCLUDES := -I ../oopt-tai/inc

test: test.cpp libtai.so libtai-stub.so
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ -L . -ltai

libtai.so:
	ln -s ../libtai-mux.so $@

libtai-stub.so:
	$(MAKE) -C ../oopt-tai/stub/
	ln -s ../oopt-tai/stub/libtai.so $@

clean:
	-rm test
