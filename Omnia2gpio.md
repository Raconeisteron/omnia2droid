## GPIO table ##
unmapped GPIO pins for b7610 (Omnia pro):
  * GPIO\_CP\_USB\_ON
  * GPIO\_TFLASH\_EN
  * GPIO\_EAR\_SEND\_END

unmapped GPIO pins for m715 (T\*Omnia):
  * GPIO\_TA\_EN
  * GPIO\_FM\_nRST
  * GPIO\_TP10024

| **pin** | **signal name** | **i8000 (omnia 2)** | **b7610 (omnia pro)** | **i6500 (saturn)** |
|:--------|:----------------|:--------------------|:----------------------|:-------------------|
|GPA(0)   | XURXD0          |GPIO\_AP\_FLM\_RXD   |GPIO\_AP\_FLM\_RXD     | GPIO\_FLM\_RXD     |
|GPA(1)   | XUTXD0          |GPIO\_AP\_FLM\_TXD   |GPIO\_AP\_FLM\_TXD     | GPIO\_FLM\_TXD     |
|GPA(2)   | XUCTSN0         |                     | **_??GPIO\_BT\_EN??_** | GPIO\_USB\_SEL     |
|GPA(3)   | XURTSN0         |                     | **_GPIO\_VIB\_EN_**   | GPIO\_MSENSE\_RST  |
|GPA(4)   | XURXD1          |GPIO\_AP\_BT\_RXD    |GPIO\_AP\_BT\_RXD      | GPIO\_BT\_RXD      |
|GPA(5)   | XUTXD1          |GPIO\_AP\_BT\_TXD    |GPIO\_AP\_BT\_TXD      | GPIO\_BT\_TXD      |
|GPA(6)   | XUCTSN1         |GPIO\_AP\_BT\_CTS    |GPIO\_AP\_BT\_CTS      | GPIO\_BT\_CTS      |
|GPA(7)   | XURTSN1         |GPIO\_AP\_BT\_RTS    |GPIO\_AP\_BT\_RTS      | GPIO\_BT\_RTS      |
|GPB(0)   | XURXD2          |GPIO\_AP\_RXD        |GPIO\_AP\_RXD          | GPIO\_PDA\_RXD     |
|GPB(1)   | XUTXD2          |GPIO\_AP\_TXD        |GPIO\_AP\_TXD          | GPIO\_PDA\_TXD     |
|GPB(2)   | XURXD3          |GPIO\_CAM\_SCL       |GPIO\_CAM\_SCL         | GPIO\_I2C1?SCL     |
|GPB(3)   | XUTXD3          |GPIO\_CAM\_SDA       |GPIO\_CAM\_SDA         | GPIO\_I2C1?SDA     |
|GPB(4)   | XIRSDBW         |GPIO\_TFLASH\_EN     | **_GPIO\_CAM\_3M\_nSTBY_** | GPIO\_TOUCH\_EN    |
|GPB(5)   | XI2CSCL         |GPIO\_AP\_SCL        |GPIO\_AP\_SCL          | GPIO\_I2C0?SCL     |
|GPB(6)   | XI2CSDA         |GPIO\_AP\_SDA        |GPIO\_AP\_SDA          | GPIO\_I2C0?SDA     |
|GPB(7)   |                 |                     |                       |                    |
|GPC(0)   | XSPIMISO0       |GPIO\_TP10024        |GPIO\_TP10024          |                    |
|GPC(1)   | XSPICLK0        |                     | **_GPIO\_CAM\_CIF\_nRST_** | GPIO\_PM\_SET1     |
|GPC(2)   | XSPIMOSI0       |                     | **_GPIO\_TVOUT\_SEL_** | GPIO\_PM\_SET2     |
|GPC(3)   | XSPICS0         |                     | **_GPIO\_MSENSE\_RST_** | GPIO\_PM\_SET3     |
|GPC(4)   | XSPIMISO1/XmmcCMD2|GPIO\_WLAN\_SPI\_RXD |GPIO\_WLAN\_SPI\_RXD   | GPIO\_WLAN\_CMD    |
|GPC(5)   | XSPICLK1/XmmcCLK2|GPIO\_WLAN\_SPI\_CLK |GPIO\_WLAN\_SPI\_CLK   | GPIO\_WLAN\_CLK    |
|GPC(6)   | XSPIMOSI1       |GPIO\_WLAN\_SPI\_TXD |GPIO\_WLAN\_SPI\_TXD   | GPIO\_WLAN\_WAKE   |
|GPC(7)   | XSPICS1         |GPIO\_WLAN\_SPI\_nCS |GPIO\_WLAN\_SPI\_nCS   | GPIO\_BT\_WAKE     |
|GPD(0)   | XPCMDCLK0       |GPIO\_I2S\_SCLK      |GPIO\_I2S\_SCLK        | GPIO\_I2S\_CLK     |
|GPD(1)   | XPCMEXTCLK0     |                     | **_GPIO\_CAM\_CIF\_nSTBY_** | GPIO\_BT\_WLAN\_REG\_ON|
|GPD(2)   | XPCMFSYNC0      |GPIO\_I2S\_SYNC      |GPIO\_I2S\_SYNC        | GPIO\_I2S\_LRCLK   |
|GPD(3)   | XPCMSIN0        |GPIO\_I2S\_SDI       |GPIO\_I2S\_SDI         | GPIO\_I2S\_DI      |
|GPD(4)   | XPCMSOUT0       |GPIO\_I2S\_SDO       |GPIO\_I2S\_SDO         | GPIO\_I2S\_DO      |
|GPE(0)   | XPCMDCLK1       |GPIO\_BT\_nRST       | **_??GPIO\_WLAN\_nRST??_** | GPIO\_BT\_RST\_N   |
|GPE(1)   | XPCMEXTCLK1     |GPIO\_BOOT\_MODE     |GPIO\_BOOT\_MODE       | GPIO\_BOOT         |
|GPE(2)   | XPCMFSYNC1      |GPIO\_WLAN\_nRST     |GPIO\_WLAN\_nRST       | GPIO\_WLAN\_RST\_N |
|GPE(3)   | XPCMSIN1        |GPIO\_PWR\_I2C\_SCL  |GPIO\_PWR\_I2C\_SCL    | GPIO\_PWR\_I2C\_SCL|
|GPE(4)   | XPCMSOUT1       |GPIO\_PWR\_I2C\_SDA  |GPIO\_PWR\_I2C\_SDA    | GPIO\_PWR\_I2C\_SDA|
|GPF(0)   | XCICLK          |GPIO\_CAM\_MCLK      |GPIO\_CAM\_MCLK        | GPIO\_CAM\_MCLK    |
|GPF(1)   | XCIHREF         |GPIO\_CAM\_HSYNC     |GPIO\_CAM\_HSYNC       | GPIO\_CAM\_HSYNC   |
|GPF(2)   | XCIPCLK         |GPIO\_CAM\_PCLK      |GPIO\_CAM\_PCLK        | GPIO\_CAM\_PCLK    |
|GPF(3)   | XCIRSTN         |GPIO\_CAM\_3M\_nRST  |GPIO\_CAM\_3M\_nRST    | GPIO\_MCAM\_RST\_N |
|GPF(4)   | XCIVSYNC        |GPIO\_CAM\_VSYNC     |GPIO\_CAM\_VSYNC       | GPIO\_CAM\_VSYNC   |
|GPF(5)   | XCIYDATA0       |GPIO\_CAM\_D\_0      |GPIO\_CAM\_D\_0        | GPIO\_CAM\_D\_0    |
|GPF(6)   | XCIYDATA1       |GPIO\_CAM\_D\_1      |GPIO\_CAM\_D\_1        | GPIO\_CAM\_D\_1    |
|GPF(7)   | XCIYDATA2       |GPIO\_CAM\_D\_2      |GPIO\_CAM\_D\_2        | GPIO\_CAM\_D\_2    |
|GPF(8)   | XCIYDATA3       |GPIO\_CAM\_D\_3      |GPIO\_CAM\_D\_3        | GPIO\_CAM\_D\_3    |
|GPF(9)   | XCIYDATA4       |GPIO\_CAM\_D\_4      |GPIO\_CAM\_D\_4        | GPIO\_CAM\_D\_4    |
|GPF(10)  | XCIYDATA5       |GPIO\_CAM\_D\_5      |GPIO\_CAM\_D\_5        | GPIO\_CAM\_D\_5    |
|GPF(11)  | XCIYDATA6       |GPIO\_CAM\_D\_6      |GPIO\_CAM\_D\_6        | GPIO\_CAM\_D\_6    |
|GPF(12)  | XCIYDATA7       |GPIO\_CAM\_D\_7      |GPIO\_CAM\_D\_7        | GPIO\_CAM\_D\_7    |
|GPF(13)  | XPWMECLK        |                     |                       |                    |
|GPF(14)  | XPWMTOUT0       |                     |                       |                    |
|GPF(15)  | XPWMTOUT1       | GPIO\_VIB\_PWM      | GPIO\_VIB\_PWM        | GPIO\_VIBTONE\_PWM |
|GPG(0)   | XMMCCLK0        |GPIO\_T\_FLASH\_CLK  |GPIO\_T\_FLASH\_CLK    | GPIO\_TF\_CLK      |
|GPG(1)   | XMMCCMD0        |GPIO\_T\_FLASH\_CMD  |GPIO\_T\_FLASH\_CMD    | GPIO\_TF\_CMD      |
|GPG(2)   | XMMCDATA0\_0    |GPIO\_T\_FLASH\_D0   |GPIO\_T\_FLASH\_D0     | GPIO\_TF\_D\_0     |
|GPG(3)   | XMMCDATA0\_1    |GPIO\_T\_FLASH\_D1   |GPIO\_T\_FLASH\_D1     | GPIO\_TF\_D\_1     |
|GPG(4)   | XMMCDATA0\_2    |GPIO\_T\_FLASH\_D2   |GPIO\_T\_FLASH\_D2     | GPIO\_TF\_D\_2     |
|GPG(5)   | XMMCDATA0\_3    |GPIO\_T\_FLASH\_D3   |GPIO\_T\_FLASH\_D3     | GPIO\_TF\_D\_3     |
|GPG(6)   | XMMCCDN0        |                     |                       |                    |
|GPH(0)   | XMMCCLK1        |GPIO\_NAND\_CLK      |GPIO\_NAND\_CLK        | GPIO\_TOUCH\_I2C\_SCL|
|GPH(1)   | XMMCCMD1        |GPIO\_NAND\_CMD      |GPIO\_NAND\_CMD        | GPIO\_TOUCH\_I2C\_SDA|
|GPH(2)   | XMMCDATA1\_0    |GPIO\_NAND\_D0       |GPIO\_NAND\_D0         | GPIO\_FM\_I2C\_SCL |
|GPH(3)   | XMMCDATA1\_1    |GPIO\_NAND\_D1       |GPIO\_NAND\_D1         | GPIO\_FM\_I2C\_SDA |
|GPH(4)   | XMMCDATA1\_2    |GPIO\_NAND\_D2       |GPIO\_NAND\_D2         | GPIO\_VIB\_EN      |
|GPH(5)   | XMMCDATA1\_3    |GPIO\_NAND\_D3       |GPIO\_NAND\_D3         |                    |
|GPH(6)   | XMMCDATA1\_4/XmmcDAT2\_0|                     |                       | GPIO\_WLAN\_D\_0   |
|GPH(7)   | XMMCDATA1\_5/XmmcDAT2\_1|                     |                       | GPIO\_WLAN\_D\_1   |
|GPH(8)   | XMMCDATA1\_6/XmmcDAT2\_2|                     |                       | GPIO\_WLAN\_D\_2   |
|GPH(9)   | XMMCDATA1\_7/XmmcDAT2\_3|                     |                       | GPIO\_WLAN\_D\_3   |
|GPI(0)   | XVVD0           |GPIO\_LCD\_D0        |GPIO\_LCD\_D0          | GPIO\_LCD\_B\_0    |
|GPI(1)   | XVVD1           |GPIO\_LCD\_D1        |GPIO\_LCD\_D1          | GPIO\_LCD\_B\_1    |
|GPI(2)   | XVVD2           |GPIO\_LCD\_D2        |GPIO\_LCD\_D2          | GPIO\_LCD\_B\_2    |
|GPI(3)   | XVVD3           |GPIO\_LCD\_D3        |GPIO\_LCD\_D3          | GPIO\_LCD\_B\_3    |
|GPI(4)   | XVVD4           |GPIO\_LCD\_D4        |GPIO\_LCD\_D4          | GPIO\_LCD\_B\_4    |
|GPI(5)   | XVVD5           |GPIO\_LCD\_D5        |GPIO\_LCD\_D5          | GPIO\_LCD\_B\_5    |
|GPI(6)   | XVVD6           |GPIO\_LCD\_D6        |GPIO\_LCD\_D6          | GPIO\_LCD\_B\_6    |
|GPI(7)   | XVVD7           |GPIO\_LCD\_D7        |GPIO\_LCD\_D7          | GPIO\_LCD\_B\_7    |
|GPI(8)   | XVVD8           |GPIO\_LCD\_D8        |GPIO\_LCD\_D8          | GPIO\_LCD\_G\_0    |
|GPI(9)   | XVVD9           |GPIO\_LCD\_D9        |GPIO\_LCD\_D9          | GPIO\_LCD\_G\_1    |
|GPI(10)  | XVVD10          |GPIO\_LCD\_D10       |GPIO\_LCD\_D10         | GPIO\_LCD\_G\_2    |
|GPI(11)  | XVVD11          |GPIO\_LCD\_D11       |GPIO\_LCD\_D11         | GPIO\_LCD\_G\_3    |
|GPI(12)  | XVVD12          |GPIO\_LCD\_D12       |GPIO\_LCD\_D12         | GPIO\_LCD\_G\_4    |
|GPI(13)  | XVVD13          |GPIO\_LCD\_D13       |GPIO\_LCD\_D13         | GPIO\_LCD\_G\_5    |
|GPI(14)  | XVVD14          |GPIO\_LCD\_D14       |GPIO\_LCD\_D14         | GPIO\_LCD\_G\_6    |
|GPI(15)  | XVVD15          |GPIO\_LCD\_D15       |GPIO\_LCD\_D15         | GPIO\_LCD\_G\_7    |
|GPJ(0)   | XVVD16          |GPIO\_LCD\_D16       |GPIO\_LCD\_D16         | GPIO\_LCD\_R\_0    |
|GPJ(1)   | XVVD17          |GPIO\_LCD\_D17       |GPIO\_LCD\_D17         | GPIO\_LCD\_R\_1    |
|GPJ(2)   | XVVD18          |GPIO\_LCD\_D18       |GPIO\_LCD\_D18         | GPIO\_LCD\_R\_2    |
|GPJ(3)   | XVVD19          |GPIO\_LCD\_D19       |GPIO\_LCD\_D19         | GPIO\_LCD\_R\_3    |
|GPJ(4)   | XVVD20          |GPIO\_LCD\_D20       |GPIO\_LCD\_D20         | GPIO\_LCD\_R\_4    |
|GPJ(5)   | XVVD21          |GPIO\_LCD\_D21       |GPIO\_LCD\_D21         | GPIO\_LCD\_R\_5    |
|GPJ(6)   | XVVD22          |GPIO\_LCD\_D22       |GPIO\_LCD\_D22         | GPIO\_LCD\_R\_6    |
|GPJ(7)   | XVVD23          |GPIO\_LCD\_D23       |GPIO\_LCD\_D23         | GPIO\_LCD\_R\_7    |
|GPJ(8)   | XVHSYNC         |GPIO\_LCD\_HSYNC     |GPIO\_LCD\_HSYNC       | GPIO\_LCD\_HSYNC   |
|GPJ(9)   | XVVSYNC         |GPIO\_LCD\_VSYNC     |GPIO\_LCD\_VSYNC       | GPIO\_LCD\_VSYNC   |
|GPJ(10)  | XVDEN           |GPIO\_LCD\_DE        |GPIO\_LCD\_DE          | GPIO\_LCD\_DE      |
|GPJ(11)  | XVVCLK          |GPIO\_LCD\_MCLK      |GPIO\_LCD\_MCLK        | GPIO\_LCD\_CLK     |
|GPK(0)   | XHIDATA0        |GPIO\_TA\_EN         |GPIO\_TA\_EN           | GPIO\_CHG\_EN      |
|GPK(1)   | XHIDATA1        |GPIO\_AUDIO\_EN      |GPIO\_AUDIO\_EN        | GPIO\_AUDIO\_EN    |
|GPK(2)   | XHIDATA2        |GPIO\_TVOUT\_SEL     | **_GPIO\_FLM\_SEL_**  | GPIO\_PHONE\_ON    |
|GPK(3)   | XHIDATA3        |GPIO\_MICBIAS\_EN    |GPIO\_MICBIAS\_EN      | GPIO\_MICBIAS\_EN  |
|GPK(4)   | XHIDATA4        |GPIO\_UART\_SEL      |GPIO\_UART\_SEL        | GPIO\_UART\_SEL    |
|GPK(5)   | XHIDATA5        |GPIO\_FM\_nRST       |GPIO\_FM\_nRST         | GPIO\_TOUCH\_RST   |
|GPK(6)   | XHIDATA6        |GPIO\_CAM\_EN        | **_GPIO\_PHONE\_ON_** | GPIO\_CAM\_EN      |
|GPK(7)   | XHIDATA7        |GPIO\_PHONE\_RST\_N  |GPIO\_PHONE\_RST\_N    | GPIO\_PHONE\_RST\_N|
|GPK(8)   | XHIDATA8        |GPIO\_KBR0           |GPIO\_KBR0             | GPIO\_KEYSCAN\_0   |
|GPK(9)   | XHIDATA9        |GPIO\_KBR1           |GPIO\_KBR1             | GPIO\_KEYSCAN\_1   |
|GPK(10)  | XHIDATA10       |GPIO\_KBR2           |GPIO\_KBR2             | GPIO\_KEYSCAN\_2   |
|GPK(11)  | XHIDATA11       |GPIO\_FLM\_SEL       | **_GPIO\_KBR3_**      | GPIO\_KEYSCAN\_3   |
|GPK(12)  | XHIDATA12       |GPIO\_CAM\_CIF\_nRST | **_GPIO\_KBR4_**      | GPIO\_FM\_RST\_N   |
|GPK(13)  | XHIDATA13       |GPIO\_CAM\_CIF\_nSTBY| **_GPIO\_KBR5_**      |                    |
|GPK(14)  | XHIDATA14       |GPIO\_MSENSE\_RST    | **_GPIO\_KBR6_**      |                    |
|GPK(15)  | XHIDATA15       |GPIO\_VREG\_MSMP\_26V|GPIO\_VREG\_MSMP\_26V  | GPIO\_VREG\_MSMP\_26V|
|GPL(0)   | XHIADDR0        |GPIO\_KBC0           |GPIO\_KBC0             | GPIO\_KEYSENSE\_0  |
|GPL(1)   | XHIADDR1        |GPIO\_KBC1           |GPIO\_KBC1             | GPIO\_KEYSENSE\_1  |
|GPL(2)   | XHIADDR2        |GPIO\_KBC2           |GPIO\_KBC2             | GPIO\_KEYSENSE\_2  |
|GPL(3)   | XHIADDR3        |GPIO\_CAM\_3M\_nSTBY | **_GPIO\_KBC3_**      | GPIO\_KEYSENSE\_3  |
|GPL(4)   | XHIADDR4        |GPIO\_VIB\_EN        | **_GPIO\_KBC4_**      | GPIO\_7\_LED\_EN   |
|GPL(5)   | XHIADDR5        |GPIO\_CP\_USB\_ON    | **_GPIO\_KBC5_**      | GPIO\_7\_LED\_LDO\_EN|
|GPL(6)   | XHIADDR6        |GPIO\_PHONE\_ON      | **_GPIO\_KBC6_**      |                    |
|GPL(7)   | XHIADDR7        |GPIO\_USIM\_BOOT     | GPIO\_USIM\_BOOT      | GPIO\_USIM\_BOOT   |
|GPL(8)   | XHIADDR8        |                     |                       | GPIO\_CAM\_3M\_STBY\_N|
|GPL(9)   | XHIADDR9        |GPIO\_MIC\_SEL       |GPIO\_MIC\_SEL         | GPIO\_MIC\_SEL\_N  |
|GPL(10)  | XHIADDR10       |GPIO\_FM\_INT        |GPIO\_FM\_INT          | GPIO\_FM\_INT      |
|GPL(11)  | XHIADDR11       |GPIO\_TA\_nCONNECTED |GPIO\_TA\_nCONNECTED   | GPIO\_TA\_CONNECTED\_N|
|GPL(12)  | XHIADDR12       |GPIO\_PS\_VOUT       |GPIO\_PS\_VOUT         | GPIO\_TOUCH\_INT\_N|
|GPL(13)  | XHIDATA16       |GPIO\_ALS\_EN        |GPIO\_ALS\_EN          | GPIO\_CP\_BOOT\_SEL|
|GPL(14)  | XHIDATA17       |GPIO\_BT\_HOST\_WAKE |GPIO\_BT\_HOST\_WAKE   | GPIO\_BT\_HOST\_WAKE|
|GPM(0)   | XHICSN          |GPIO\_FM\_SCL        |GPIO\_FM\_SCL          |                    |
|GPM(1)   | XHICSN\_MAIN    |GPIO\_FM\_SDA        |GPIO\_FM\_SDA          |                    |
|GPM(2)   | XHICSN\_SUB     |GPIO\_TA\_nCHG       |GPIO\_TA\_nCHG         | GPIO\_CHG\_ING\_N  |
|GPM(3)   | XHIWEN          |GPIO\_PDA\_ACTIVE    |GPIO\_PDA\_ACTIVE      | GPIO\_PDA\_ACTIVE  |
|GPM(4)   | XHIOEN          |GPIO\_AMBIENT\_INT\_N|GPIO\_AMBIENT\_INT\_N  |                    |
|GPM(5)   | XHIINTR         |GPIO\_TP10025        |GPIO\_TP10025          |                    |
|GPN(0)   | XEINT0          |GPIO\_nONED\_INT\_AP |GPIO\_nONED\_INT\_AP   | GPIO\_ONEDRAM\_INT\_N|
|GPN(1)   | XEINT1          |GPIO\_HOST\_WAKE     |GPIO\_HOST\_WAKE       | GPIO\_WLAN\_HOST\_WAKE|
|GPN(2)   | XEINT2          |GPIO\_MSENSE\_IRQ    |GPIO\_MSENSE\_IRQ      | GPIO\_MSENSE\_INT  |
|GPN(3)   | XEINT3          |GPIO\_ACC\_INT       |GPIO\_ACC\_INT         | GPIO\_ACC\_INT     |
|GPN(4)   | XEINT4          |GPIO\_BT\_EN         | **_??GPIO\_BT\_nRST??_** | GPIO\_SIM\_DETECT\_N|
|GPN(5)   | XEINT5          |GPIO\_nPOWER         |GPIO\_nPOWER           | GPIO\_POWER\_N     |
|GPN(6)   | XEINT6          |GPIO\_T\_FLASH\_DETECT|GPIO\_T\_FLASH\_DETECT | GPIO\_TF\_DETECT   |
|GPN(7)   | XEINT7          |GPIO\_PHONE\_ACTIVE\_AP|GPIO\_PHONE\_ACTIVE\_AP| GPIO\_PHONE\_ACTIVE|
|GPN(8)   | XEINT8          |GPIO\_PM\_INT\_N     |GPIO\_PM\_INT\_N       | GPIO\_PMIC\_INT\_N |
|GPN(9)   | XEINT9          |GPIO\_INTB           |GPIO\_INTB             | GPIO\_JACK\_INT\_N |
|GPN(10)  | XEINT10         |GPIO\_DET\_35        |GPIO\_DET\_35          | GPIO\_DET\_35      |
|GPN(11)  | XEINT11         |GPIO\_EAR\_SEND\_END | **_GPIO\_HALL\_SW_**  | GPIO\_EAR\_SEND\_END|
|GPN(12)  | XEINT12         |GPIO\_RESOUT\_N\_AP  |GPIO\_RESOUT\_N\_AP    | GPIO\_RESOUT\_N    |
|GPN(13)  | XEINT13         |GPIO\_BOOT\_EINT13   |GPIO\_BOOT\_EINT13     | GPIO\_BOOT\_EINT13 |
|GPN(14)  | XEINT14         |GPIO\_BOOT\_EINT14   |GPIO\_BOOT\_EINT14     | GPIO\_BOOT\_EINT14 |
|GPN(15)  | XEINT15         |GPIO\_BOOT\_EINT15   |GPIO\_BOOT\_EINT15     | GPIO\_BOOT\_EINT15 |
|GPO(0)   | Xm0ADDR0        |GPIO\_AP\_NANDCS     |GPIO\_AP\_NANDCS       | GPIO\_NAND\_CS\_N  |
|GPO(1)   | Xm0ADDR1        |                     |                       |                    |
|GPO(2)   | Xm0ADDR2        |GPIO\_MLCD\_RST\_N   |GPIO\_MLCD\_RST\_N     | GPIO\_LCD\_RST\_N  |
|GPO(3)   | Xm0ADDR3        |                     |                       |                    |
|GPO(4)   | Xm0ADDR4        |                     |                       |                    |
|GPO(5)   | Xm0ADDR5        |                     |                       |                    |
|GPO(6)   | Xm0ADDR6        |GPIO\_DISPLAY\_CS    |GPIO\_DISPLAY\_CS      | GPIO\_LCD\_CS\_N   |
|GPO(7)   | Xm0ADDR7        |GPIO\_DISPLAY\_SI    |GPIO\_DISPLAY\_SI      | GPIO\_LCD\_SDI     |
|GPO(8)   | Xm0ADDR8        |                     |                       |                    |
|GPO(9)   | Xm0ADDR9        |                     |                       |                    |
|GPO(10)  | Xm0ADDR10       |                     |                       |                    |
|GPO(11)  | Xm0ADDR11       |                     |                       |                    |
|GPO(12)  | Xm0ADDR12       |GPIO\_LCD\_VHG       |GPIO\_LCD\_VHG         | GPIO\_LCD\_ID // GPIO\_LCD\_SDO|
|GPO(13)  | Xm0ADDR13       |GPIO\_DISPLAY\_CLK   |GPIO\_DISPLAY\_CLK     | GPIO\_PDA\_PS\_HOLD // GPIO\_LCD\_SCLK|
|GPO(14)  | Xm0ADDR14       |GPIO\_LCD\_ID        |GPIO\_LCD\_ID          |                    |
|GPO(15)  | Xm0ADDR15       |GPIO\_TV\_EN         |GPIO\_TV\_EN           |                    |
|GPP(1)   |                 |                     | **_???_**             |                    |
|GPP(5)   |                 |                     | **_???_**             |                    |
|GPP(13)  | Xm0OEata        |GPIO\_PS\_HOLD\_PDA  |GPIO\_PS\_HOLD\_PDA    |                    |
|GPQ(5)   |                 |                     | **_GPIO\_CAM\_EN_**   |                    |