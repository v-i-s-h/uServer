uServer
-------

This project aims at developing a full HTTP server around TI's Connect Launchpad TM4C1294XL.
The following are the features of the server :
<br/>1. Full HTTP communication
<br/>2. Read and serve files from the attached USB pendrive/harddisk.
<br/>3. Control the on board peripherals over HTTP
	
The following opensource projects are used in this project
<br/>1. lwIP
<br/>2. FatFs

Hardware
--------
This project is intended to run on EK-TM4C1294XL from TI. This evaluation kit has a 32bit ARM Cortex M4 core, 1MB flash, 256kB SRAM among other features.

License
-------
This project is distributed under The MIT license unless and otherwise specified in the source. Some of the code used in this project is derived from tivaware from Texas Instruments, and their license is mentioned in those source files. I'm using other open source projects lwIP and FatFs -- their source code is also included with this project mentioning their license terms. In short, If you find any source file with no license mentioned, it will be by default MIT license.
