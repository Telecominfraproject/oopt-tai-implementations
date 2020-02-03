SFF TAI implementation
===

SFF TAI library ( `libtai-sff.so` ) is a TAI library to support SFF transceivers
like SFP, QSFP+, QSFP28. It uses the [optoe](https://github.com/opencomputeproject/oom/tree/master/optoe) driver to
access EEPROM in the transceivers.

The location used to identify the transceiver is the sysfs directory which the optoe driver creates.

```
> list
module: /sys/bus/i2c/devices/18-0050 0x1000000000009
 hostif: 0 0x2000000000900
 netif: 0 0x3000000000900
 netif: 1 0x3000000000901
 netif: 2 0x3000000000902
 netif: 3 0x3000000000903
module: /sys/bus/i2c/devices/19-0050 not present
module: /sys/bus/i2c/devices/20-0050 not present
module: /sys/bus/i2c/devices/21-0050 not present
module: /sys/bus/i2c/devices/22-0050 0x1000000000001
 hostif: 0 0x2000000000100
 netif: 0 0x3000000000100
 netif: 1 0x3000000000101
 netif: 2 0x3000000000102
 netif: 3 0x3000000000103
module: /sys/bus/i2c/devices/23-0050 not present
module: /sys/bus/i2c/devices/24-0050 not present
module: /sys/bus/i2c/devices/25-0050 not present
module: /sys/bus/i2c/devices/26-0050 not present
module: /sys/bus/i2c/devices/27-0050 0x1000000000005
 hostif: 0 0x2000000000500
 netif: 0 0x3000000000500
 netif: 1 0x3000000000501
 netif: 2 0x3000000000502
 netif: 3 0x3000000000503
module: /sys/bus/i2c/devices/28-0050 not present
module: /sys/bus/i2c/devices/29-0050 not present
module: /sys/bus/i2c/devices/30-0050 not present
module: /sys/bus/i2c/devices/31-0050 not present
module: /sys/bus/i2c/devices/32-0050 not present
module: /sys/bus/i2c/devices/33-0050 not present
module: /sys/bus/i2c/devices/34-0050 not present
module: /sys/bus/i2c/devices/35-0050 not present
module: /sys/bus/i2c/devices/36-0050 not present
module: /sys/bus/i2c/devices/37-0050 not present
module: /sys/bus/i2c/devices/38-0050 not present
module: /sys/bus/i2c/devices/39-0050 not present
module: /sys/bus/i2c/devices/40-0050 not present
module: /sys/bus/i2c/devices/41-0050 not present
module: /sys/bus/i2c/devices/42-0050 not present
module: /sys/bus/i2c/devices/43-0050 not present
module: /sys/bus/i2c/devices/44-0050 not present
module: /sys/bus/i2c/devices/45-0050 not present
module: /sys/bus/i2c/devices/46-0050 not present
module: /sys/bus/i2c/devices/47-0050 not present
module: /sys/bus/i2c/devices/48-0050 not present
module: /sys/bus/i2c/devices/49-0050 not present
>
> module /sys/bus/i2c/devices/18-0050
module(/sys/bus/i2c/devices/18-0050)> get power
3.258300
module(/sys/bus/i2c/devices/18-0050)> get temp
25.535156
module(/sys/bus/i2c/devices/18-0050)> netif 0
module(/sys/bus/i2c/devices/18-0050)/netif(0)> get current-output-power
1.388393
module(/sys/bus/i2c/devices/18-0050)/netif(0)> get current-input-power
1.428273
module(/sys/bus/i2c/devices/18-0050)/netif(0)>
```

### HOW TO BUILD

```
$ git submodule update --init
$ make docker-image
$ make docker
```

### Licensing
`libtai-sff.so` is licensed under the Apache License, Version 2.0. See LICENSE for the full license text.
