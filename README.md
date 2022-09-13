# hUCCB
uUCCB is a modified version of [USBCanConverterBasic (UCCB)](https://github.com/UsbCANConverter-UCCbasic) which is open source USB converter for Controller Area Network (CAN). It has been adapted to the different MCU because of semiconductors shortage.

## Firmware
The original firmware comes from [ucandevices - USBCanConverterBasic(UCCB)](https://github.com/UsbCANConverter-UCCbasic/UCCBEmbedded) and has been rewritten for STM32F072CB MCU. The main changes made in the hUCCB are the MCU change and the implementation of the DMA on the UART interface.

## Hardware
The PCBA is also based on the original UCCB - see the original project on Github: [UCCBPCB](https://github.com/UsbCANConverter-UCCbasic/UCCBPCB). We include the PCB design files for hUCCB in the **PCB** folder. The main changes are the MCU change and USB connector change. We have also added a crystal to make the asynchronous serial interfaces  more stable.

## MCU Flashing
To program the MCU use [CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html), [ST-Link Utility](https://www.st.com/en/development-tools/stsw-link004.html) and 
select a binary found in **Debug** folder or download [Atollic TrueSTUDIO](https://www.st.com/en/development-tools/truestudio.html) and compile the code yourself. 
 
