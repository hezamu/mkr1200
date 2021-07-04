# Arduino MKR1200 test project

Sends the status code of previous attempt and the board internal temperature to the SigFox network every 15 minutes. 3 payload bytes of the max 12 are currently used.

Note that using `LowPower.deepSleep()` means that the board will shut down the MCU shortly after powerup, making uploading new versions difficult. To solve this, press the reset button twice in fast succession to put the board into a bootloader mode on a fixed COM port.
