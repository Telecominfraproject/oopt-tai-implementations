# oopt-tai-implementations

This repo contains implementations of the Transponder Abstraction Interface, or 
TAI, which have been contributed to the Telecom Infrastructure Project, or TIP.

In summary, TAI is a user-mode device driver interface for transponder modules 
based upon the Switch Abstraction Interface, or SAI, of the Open Compute 
Project, or OCP. Although it is not required that the device drivers be 
open-sourced, some vendors have opened up their code for the benefit of the 
community.

Each open sourced component is in a sub-directory of this repo. See the 
README.md file in each subdirectory for more information about the component and 
build instructions. The repo currently contains the following components:

* tai_ac400 - The TAI adapter code for the Acacia AC400 module
* tai_mux - A TAI adapter which multiplexes access to multiple, possible different, TAI adapters.
* tai_sff - A TAI adapter code for SFF transceiver

For more information about:
* TAI: https://github.com/Telecominfraproject/oopt-tai
* TIP: https://telecominfraproject.com/
* SAI: https://github.com/opencomputeproject/SAI
* OCP: https://www.opencompute.org/

