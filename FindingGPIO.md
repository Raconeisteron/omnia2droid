# Finding GPIO pins to support other phones simular to I8000 #

## Introduction ##
GPIO means General Purpose Input Output, and is used by a microprocessors to communicate to other logic / IC's.
The Samsung I8000 phone (Omnia 2) has a s3c6410 processor with 187 PIO pins grouped by 17 ports (A to Q).
Almost all GPIO pins for the I8000 have been found and listed on [Omnia2gpio](http://code.google.com/p/omnia2droid/wiki/Omnia2gpio).
Some I8000 GPIO pins may have not been found or are listed/defined wrong. This could cause (parts) of Android not to function as intended.

## Hardware differences in Omnia 2 phone family ##
There are many Samsung Windows Mobile phones simular to I8000 but differ on some hardware.
These phones can use the Android port of the I8000 but not all functions work properly.
To make Android work completely on these phone the hardware differences have to be discovered and patched in the linux kernel.
Discovering can be done reading service manuals, detailed schematics and internet sources.

Hardware differences we already know:

|          | **B7610**                                   | **I8000**           |
|:---------|:--------------------------------------------|:--------------------|
| keyboard | hardware QWERTY keyboard and 9 phone keys   | only 9 phone keys   |
| wifi/BT/FM | SWB-T30 chip for wifi/bt/radio connected through SPI for WLAN ( UARTI for BT) with WL1271 driver | SWB-M20 chip for wifi connected through spi with libertas driver |

|          | **I920**                                   | **I8000/B7610**           |
|:---------|:-------------------------------------------|:--------------------------|
| phone    | CMDS phone part with ?? chip connected by ?? and driven by ?? driver |  GSM phone part with Qualcomm MSM6290 for I800 and probably MSM6246 for B7610 |

## Reverse engineering tools ##
Sometimes detailed hardware info is not available so reverse engineering is needed.
There are programs on Windows Mobile you can run (eg GPIOgfx, HaRET) so you can monitor memory and gpio pins while you use your winmo phone.
HaRET is known for booting linux from winmo, but when tapped on "Listen" you can send commands to the network connection on your phone.
You can use Telnet to issue HaRET GPIO commands, or use linux scripts. HaRET is an expert tool and usefull if you want some more detail in timing.
GPIOgfx is an easy to use program to detect GPIO pins, and can be used for more getting a clue where to find things with HaRET.

## GPIOgfx ##
Some tips using the [GPIOgfx](http://tanguy.ath.cx/index.php?q=english/content/gpiogfx-s3clock-s3c64x0-projects) program:
  * tap on right upper screen to switch from other other modes (PULLED PINS, SLEEP MODE, SLEEP PULL) to LEVEL CHANGE mode
  * tap on screens below to capture the screen above
  * menu/reset level change to make a blank LEVEL CHANGE screen
  * menu/ignore-changed-pins to exclude pins like I and J LCD screen GPIO ports


![https://lh3.googleusercontent.com/_HCfaazcXikU/TUqOOzkVhEI/AAAAAAAAABs/EK0Qpr_nNfU/s0/gpiogfx-screen.png](https://lh3.googleusercontent.com/_HCfaazcXikU/TUqOOzkVhEI/AAAAAAAAABs/EK0Qpr_nNfU/s0/gpiogfx-screen.png)

## How to help finding GPIO pins ##
  * install [GPIOgfx](http://tanguy.ath.cx/index.php?q=system/files/gpiogfx.CAB) winmo app and needed [framework lib](ftp://ftp2.pcsoft.com/DLL/14.0/US/InstallFramework_ARM.cab) on your phone
  * play a bit with the tool to so you can use its functions
  * try described winmo actions and report found GPIO pins not already reported
  * find better winmo actions which trigger less GPIO pins at the same time
  * if you got detailed info on hardware not listed pse comment or mail

## Found and Mapped GPIO pins ##
When a GPIO pin is Found using GPIOgfx, a "F" is set on it's grid.
If the function of the GPIO can be guessed or is Mapped already "M" is set on the grid.

I8000 (Omnia 2)
|   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | 1 | 2 | 3 | 4 | 5 |
|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|
| A |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| B |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| C |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| D | F |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| E |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| F | F | F | F | F | F | F | F | F | F | F | F | F | F |   |   |   |
| G |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| H |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| I | F | F | F | F | F | F | F | F | F | F | F | F | F | F | F | F |
| J | F | F | F | F | F | F | F | F | F | F | F | F |   |   |   |   |
| K |   |   |   |   |   |   |   |   | F | F | F |   |   |   |   |   |
| L |   | F | F | F |   |   |   |   |   |   |   |   |   |   |   |   |
| M |   |   |   |   | F |   |   |   |   |   |   |   |   |   |   |   |
| N |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| O |   |   |   |   |   |   |   | F |   |   |   |   |   |   |   |   |
| P |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| Q |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |

B7610 (Omnia pro)
|   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | 1 | 2 | 3 | 4 | 5 |
|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|
| A |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| B |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| C |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| D |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| E |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| F |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| G |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| H |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| I |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| J |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| K |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| L |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| M |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| N |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| O |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| P |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| Q |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |

I920 (Omnia 2 verizon)
|   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | 1 | 2 | 3 | 4 | 5 |
|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|
| A |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| B |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| C |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| D |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| E |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| F |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| G |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| H |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| I |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| J |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| K |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| L |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| M |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| N |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| O |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| P |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
| Q |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |

## Found winmo actions and matching GPIO pins ##

Notes:
  * RB=Redcorder, BB=Bluecorder, WB=Whitecorder, RC=Redcube, BC=Bluecube, WC=Whitecube, OC=Orangecube with red border
  * pse just ignore the (I0-15WB)(J0-11WB) pins, they are for the screen/LCD
  * pse first turn phone/wifi/bt/sound off to have minimum basic activity
  * pse read from right upper screen (LEVEL CHANGE) or tap or it to see right screen
  * things like "plug in headphone, choose TV-OUT" can be risky try only if you like ...
  * try the action multiple times and report when reaction is the same each time

| **phone TYPE** | **#** | **found GPIO pins** | **action(s) on winmo** |
|:---------------|:------|:--------------------|:-----------------------|
| I8000          | 001   | (D0WB)(D2WB)(M4WB)(O7RB)| touch winmo status bar right upper corner twice |
| I8000          | 002   | (K10BB)(L1WB)       | press VOLUP key        |
| I8000          | 003   | (K10BB)(L2WB)       | press VOLDOWN key      |
| I8000          | 017   | (K10BB)(L2WB)(L4RB)(F15OC) | hold VOLDOWN key (vibrates) |
| I8000          | 004   | (K8BB)(L1WB)        | press CALL key         |
| I8000          | 005   | (K8BB)(L2WB)        | press MENU key         |
| I8000          | 006   | (N5WB)              | press HANGUP key       |
| I8000          | 007   | (K9BB)(L2WB)        | halfpress CAM key      |
| I8000          | 008   | (K9BB)(L3RB) (F0-12WC) and some more       | deeppress CAM key and press X to close cam |
| I8000          | 009   | ??                  | press POWER key, wait and POWER key agian |
| I8000          | 010   | ??                  | press status bar and set WIFI on |
| I8000          | 012   | ??                  | press status bar and set BT on  |
| I8000          | 013   | ??                  | remove headphone       |
| I8000          | 014   | ??                  | plug in headphone, choose headphone  |
| I8000          | 015   | ??                  | plug in headphone, choose headset, remove headphone  |
| I8000          | 016   | ??                  | plug in headphone, choose TV-OUT, remove headphone  |