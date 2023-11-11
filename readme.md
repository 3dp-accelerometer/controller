Description
-----------

This firmware acts as proxy in between host and accelerometer.
It fetches sampled acceleration data from ADXL345 in a real-time manner by using a Blackpill board (ARM STM32F401).
The proxy asserts no sample is lost and each measurement is sampled equidistant.
The project is set up with PlatformIO

1. (coding, compiling, upload),
2. CubeMx generator (for resource planing and device initialization) and
3. stm32pio as glue in-between CubeMX code generator and PIO.

The host to controller communication takes place by Virtual Com port in CDC mode.
For more details refer to the `host-api` repository.
It provides the python API to communicate with the controller which can work as standalone script as well.
The CLI interface allows to manipulate the controller:

- start/stop sampling (in streaming mode or up to specific limit of samples)
- setup/reset device (output data rate, range, scale)
- decode samples
- print samples or store tabular in file

Prerequisites
-------------

1. install [CubeMx](https://www.st.com/en/development-tools/stm32cubemx.html)
2. install [stm32pio](https://github.com/ussserrr/stm32pio)
3. ensure the `cubemx_cmd` in stm32pio.ini is set correctly:

       [app]
       platformio_cmd = platformio
       cubemx_cmd = /home/foo/STM32CubeMX/STM32CubeMX
       java_cmd = None

Workflow
--------

1. edit `cubemx.ioc` with CubeMx, safe file
2. generate code and patch existing code: `stm32pio generate`
3. compile and run: `pio run -t upload`

Connecting Modules
------------------

```
     +---------------------optional-for-development-------------------------------+
     |                                                                            |
     |                    +-----------------+                +------------+       |
     |                    | BlackPill       |                | ADXL345    |       |
+-------+----+            | STM32F401       |                | ADXL345    |       |
| HOST       |      +-----+                 |                |            |       |
| postpro-   |--USB-| PA12| USB_OTG_FS_DP   |                |            |       |
| cessing    |      | PA11| USB_OTG_FS_DM   |                |            |       |
|            |      +-----|                 |                |            |       |
|            |            | watermark EXTI2 |--PA2-----INT1--|            |       |
|            |            |   overrun EXTI3 |--PA3-----INT2--|            |       |
+------------+            |                 |--PA4------!CS--|            |       |
                          |                 |--PA5------SCL--|            |       |
                          |                 |--PA6------SDO--|            |       |
                          |                 |--5V0------VCC--|            |       |       
                          |                 |--GND------GND--|            |       |       
                          |                 |                +------------+       |
                          |                 |                                     |
                          |                 |                +------------+       |              
                          |                 |--3V3------3V3--| STLink     +---+   |              
                          |                 |--GND------GND--| programmer |USB+---+              
                          |           SWDIO |--PA13---SWDIO--|            +---+              
                          |           SWCLK |--PA14---SWCLK--|            |              
                          |                 |                +------------+
                          |                 |                              
                          |       USER_KEY0 |--PA0--+
                          |       USER_LED0 |--PC13-+
                          |    RCC_OSC32_IN |--PC14-+
                          |   RCC_OSC32_OUT |--PC15-+
                          |                 |                              
                          |     USER_DEBUG0 |--PA1--
                          |             MCO |--PA8--
                          |                 |                              
                          +-----------------+                
```

References
----------

1. https://docs.platformio.org/en/stable/boards/ststm32/blackpill_f401cc.html
2. https://docs.platformio.org/en/stable/frameworks/stm32cube.html#framework-stm32cube
3. https://wiki-power.com/en/PlatformIO%E6%90%AD%E9%85%8DCubeMX%E9%A3%9F%E7%94%A8/#cubemx-initialization-steps
4. https://github.com/ussserrr/stm32pio
5. https://community.platformio.org/t/board-genericstm32h750vb-not-found/36441
