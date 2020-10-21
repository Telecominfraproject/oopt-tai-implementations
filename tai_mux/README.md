MUX TAI implementation
===

MUX TAI library ( `libtai-mux.so` ) is a TAI library to multiplex multiple TAI
libraries. `libtai-mux.so` can be used for hardware which supports multiple
types of optical module which supports TAI. In such hardware, each optical module
vendors provides `libtai.so` for their modules. `libtai-mux.so` dynamically loads
these library and provides a single multiplexed TAI interface to TAI adapter host.

### platform adapter

Inside `libtai-mux.so`, platform adapter detects the presence of optical
modules and decides which `libtai.so` to use for detected modules.

`libtai-mux.so` has modular design to support various types of 
platform adapter. User can choose which platform adapter to use by
passing an environment variable `TAI_MUX_PLATFORM_ADAPTER`.

Currently, `static` platform adapter and `exec` platform adapter is supported.

By default, `TAI_MUX_PLATFORM_ADAPTER` is set to `static`.

#### static platform adapter

static platform adapter is a platform adapter which uses static configuration
for module detection and `libtai.so` loading.

Here is a sample configuration.

```json
{
    "1": "libtai-a.so",
    "2": "libtai-a.so",
    "3": "libtai-a.so",
    "4": "libtai-a.so",
    "5": "libtai-b.so",
    "6": "libtai-b.so",
    "7": "libtai-b.so",
    "8": "libtai-b.so"
}
```

The format is `json`. The key is the location of the module and the value is
the library to use for it.

By using the configuration above, `libtai-a.so` is used for modules
whose location is 1,2,3,4, and `libtai-b.so` is used for modules whose location
is 5,6,7,8.

An environment variable `TAI_MUX_STATIC_CONFIG_FILE` is used to let the
static platform adapter know the location of the configuration file.

#### exec platform adapter

exec platform adapter is a platform adapter which executes a prespecified script.

User can choose what script to execute by using an environment variable `TAI_MUX_EXEC_SCRIPT`.

By default, `TAI_MUX_EXEC_SCRIPT` is set to `/etc/tai/mux/exec.sh`.

exec platform adapter executes this script in two ways.

The first way is to get the list of modules during the initialization phase.
In this case, exec platform adapter passes 'list' as the first argument.
The script must return the list of modules which is separated by a newline, or "\n".

The second way is to get the actual TAI library to use for a module when TAI adapter host tries to create a module.
In this case, exec platform adapter passes the module location as the first argument.
The script must return the TAI library name. If the module doesn't exist in the specified location, the script must exit with non-zero value.

### HOW TO BUILD

```
$ git submodule update --init
$ make
```

### HOW TO TEST

```
$ cd test
$ make
$ ./run.sh
```

### Licensing
`libtai-mux.so` is licensed under the Apache License, Version 2.0. See LICENSE for the full license text.

external library `json.hpp` which is distributed under MIT License is used in `libtai-mux.so`
