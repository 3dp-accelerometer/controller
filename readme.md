Introduction
------------

This firmware acts as proxy in between host and accelerometer.
It fetches sampled acceleration data from ADXL345 in a real-time manner by using a Blackpill board (ARM STM32F401).
The proxy asserts no sample is lost and each measurement is sampled equidistant.

```
+------------+       +------------+       +------------+
|            |       |            |       |            |
|    HOST    |--USB--| Controller |--SPI--|   Sensor   |
|            |       | (proxy)    |       |            |
+------------+       +------------+       +------------+
```

This project is realized by

1. `PlatformIO` (compile, flash),
2. `CubeMx` code generator (for resource planing and device initialization) and the
3. `stm32pio` tool as glue in-between CubeMX code generator and PlatformIO.

The host to controller communication takes place by Virtual Com port in CDC mode.
For more details refer to the host-api repository [py3dpaxxel](https://github.com/3dp-accelerometer/py3dpaxxel).
It provides the python API to communicate with this firmware which can work as standalone command line script as well.
The CLI interface allows to manipulate the controller as follows:

- start/stop sampling (in streaming mode or up to specific limit of samples)
- setup/reset device (output data rate, range, scale)
- decode samples from controller
- print samples or store in tabular separated values file

See also:
- Python Host-API [py3dpaxxel](https://github.com/3dp-accelerometer/py3dpaxxel/)
- OctoPrint plugin [Octoprint Accelerometer](https://github.com/3dp-accelerometer/octoprint-accelerometer) (work in progress)
- **Read the Docs** at [3dp-accelerometer.github.io](https://3dp-accelerometer.github.io/controller)

[![Build Test Docs](https://github.com/3dp-accelerometer/controller/actions/workflows/build-test-builddocs.yaml/badge.svg)](https://github.com/3dp-accelerometer/controller/actions/workflows/build-test-builddocs.yaml)

Prerequisites
-------------

The following steps use Poetry to install required tools and set up the environment.
This is not a must but for convenience rather.
All steps can be achieved manually without Poetry: develop, build, flash, generate documentation.

1. Install development packages
    * Manually
        1. `pip install platformio`
        2. install [stm32pio](https://github.com/ussserrr/stm32pio)
    * Poetry
       ```bash
       cd utils
       poetry shell
       poetry install
       ```
2. Install [CubeMx](https://www.st.com/en/development-tools/stm32cubemx.html)
3. ensure the `cubemx_cmd` in stm32pio.ini is set correctly:
   ```bash
   [app]
   platformio_cmd = platformio
   cubemx_cmd = /home/foo/STM32CubeMX/STM32CubeMX
   java_cmd = None
   ```

Workflow
--------

1. Setup environment
    * Poetry
    ```bash
    cd utils
    poetry shell
    poetry install
    ```
    * Manually: ensure `platformio` and `stm32pio` are visible in the environment.
2. optional: initialize project for your desired IDE; i.e. `pio init --ide clion`
3. optional: modify user code in the marked sections of the auto generated CubeMX code
4. optional: edit `cubemx.ioc` with CubeMx, safe file
5. generate code and patch existing code: `stm32pio generate`
6. optional: run native unit tests with `pio test --environment native
7. optional: iterate development by returning to step 3
8. compile and flash controller: `pio run --target upload`

If only flashing the latest firmware is your desire follow first and last step which essentially boils down to:
```bash
    git clone https://github.com/3dp-accelerometer/controller.git
    cd controller/utils
    poetry shell
    poetry install
    cd ..
    # attach stlink and controller to host as described in the subsequent section "Connecting Modules"
    pio run --target upload
```

Connecting Modules
------------------

The controller (BlackPill) is attached to the HOST (i.e. your PC, OctoPrint server, ...). 
It acts as proxy that shovels the data from the accelerometer so that each sample that is reported to the host,
is sampled in an equidistant manner.
A constant time separation in between samples is a key aspect for later Fourier transformation.

```
+------------+            +-----------------+                +------------+
| HOST       |            | BlackPill       |                | Accelero   |
|            |            | STM32F401       |                | meter      |
|            |      +-----+                 |                | ADXL345    |
| post pro-  |--USB-| PA12| USB_OTG_FS_DP   |                |            |
| cessing    |      | PA11| USB_OTG_FS_DM   |                |            |
|            |      +-----|                 |                |            |
|            |            | watermark EXTI2 |--PA2-----INT1--|            |
|            |            |   overrun EXTI3 |--PA3-----INT2--|            |
+-----+------+            |                 |--PA4------!CS--|            |
      |                   |                 |--PA5------SCL--|            |
      U                   |                 |--PA6------SDO--|            |
      S                   |                 |--5V0------VCC--|            |
      B                   |                 |--GND------GND--|            |
      | optional          |                 |                +------------+
      | for debugging     |                 |
      |                   |                 |                +------------+
      |                   |                 |--3V3------3V3--| STLink     +---+
      |                   |                 |--GND------GND--| programmer |USB|--+              
      |                   |           SWDIO |--PA13---SWDIO--|            +---+  |            
      |                   |           SWCLK |--PA14---SWCLK--|            |      |        
      |                   |                 |                +------------+      |
      |                   |       USER_KEY0 |--PA0--+                            |
      |                   |       USER_LED0 |--PC13-+                            |
      |                   |    RCC_OSC32_IN |--PC14-+                            |
      |                   |   RCC_OSC32_OUT |--PC15-+                            |
      |                   |                 |                                    |
      |                   |     USER_DEBUG0 |--PA1-- optional for debugging      |
      |                   |             MCO |--PA8-- optional for debugging      |
      |                   +-----------------+                                    |
      +--------------------------------------------------------------------------+                
```

References
----------

1. Datasheets [./datasheets](./datasheets)
2. PlatformIO [BlackPill](https://docs.platformio.org/en/stable/boards/ststm32/blackpill_f401cc.html)
3. PlatformIO [stm32cube](https://docs.platformio.org/en/stable/frameworks/stm32cube.html#framework-stm32cube) framework
4. Platformio [with CubeMX](https://wiki-power.com/en/PlatformIO%E6%90%AD%E9%85%8DCubeMX%E9%A3%9F%E7%94%A8/#cubemx-initialization-steps)
5. [stm32pio](https://github.com/ussserrr/stm32pio)
6. https://community.platformio.org/t/board-genericstm32h750vb-not-found/36441
