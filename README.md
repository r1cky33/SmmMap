# SmmMap
PoC SMM-Module that manually maps another module in SMRAM

## Introduction
SmmMap is a UEFI SMM payload loader that dynamically maps and executes an additional SMM module at runtime.

## Building

Credits to the Authors of [Plouton](https://github.com/pRain1337/plouton/tree/main/Plouton-UEFI#option-1-building-on-docker-windows-linux-os-x). They provide a docker container that can be used to build the module. 
After pulling the container, you should be able to simply run the command from the README.

## Usage

Credits to the Autor of [SmmInfect](https://github.com/Oliver-1-1/SmmInfect) for releasing SmmInfect which is used as a payload module here.
```
(I) [HELLO] SmmMap Initializing... 
(I) [INFO] Handler registered 
(I) [INFO] Trying to map payload 
(I) [INFO] Get Image information 
(I) [INFO] Allocating Memory for manual mapping image 
(I) [INFO] Image Entry returned: 0 
(I) [INFO] Grabbing relocation info 
(I) [INFO] Calling RelocateForRuntime 
(I) [INFO] Getting entrypoint 
(I) [INFO] Successfully mapped image... calling entrypoint: 0x7fee64e6 
(I) [HELLO] SmmInfect Initializing... 
(I) [INFO] SmmInfect HANDLER CALLED!(I)
```

You can inspect the serial output (QEMU in this case) to see whats going on.

## Disclaimer

This project is for educational and research purposes only.  
Running code in SMM has serious security implications. When you are flashing your systems there is always the change that it breaks. Use responsibly.

## License

MIT
