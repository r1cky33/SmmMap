# SmmMap
PoC SMM-Module that manually maps another module in SMRAM

## SmmMap
SmmMap is a UEFI SMM payload loader that dynamically maps and executes an additional SMM module at runtime.

## Building

Credits to the Authors of [Plouton](https://github.com/pRain1337/plouton/tree/main/Plouton-UEFI#option-1-building-on-docker-windows-linux-os-x). They provide a docker container that can be used to build the module. 
After pulling the repo you should be able to just run `build.sh`. (If you're on windows please follow the instructions [Windows-Build](https://github.com/pRain1337/plouton/tree/main/Plouton-UEFI#option-2-building-on-windows))

## Disclaimer

This project is for educational and research purposes only.  
Running code in SMM has serious security implications. When you are flashing your systems there is always the change that it breaks. Use responsibly.

## License

MIT
