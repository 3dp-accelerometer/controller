Description
-----------

This firmware aims at fetching acceleration data from ADXL345 in a real-time manner by using a Blackpill board (ARM STM32F401). The project is set up with PlatformIO + CubeMx generator.
The data is sent USB-CDC serial to the host where it can be captrued and post processed.

Prerequisites
-------------

1. install [CubeMx](https://www.st.com/en/development-tools/stm32cubemx.html)
2. install [stm32pio](https://github.com/ussserrr/stm32pio)
3. ensure the `cubemx_cmd` in stm32pio.ini is set correctly:

       [app]
       platformio_cmd = platformio
       cubemx_cmd = ${sysenv.HOME}/STM32CubeMX/STM32CubeMX
       java_cmd = None

Workflow
--------

1. edit `cubemx.ioc` with CubeMx, safe file
2. generate code and patch existing code: `stm32pio generate`
3. compile and run: `pio run -t upload`

Roadmap
-------

- todo command: start/stop of sampling
- todo command: configurable sampling rate, resolution and range
- todo transport: binary not ascii
- todo: assert sampling separation is equidistant
- todo: assert no data is left behind (fifo overrun) 
- todo: implement simple python api to gather data from controller

References
----------

1. https://docs.platformio.org/en/stable/boards/ststm32/blackpill_f401cc.html
2. https://docs.platformio.org/en/stable/frameworks/stm32cube.html#framework-stm32cube
3. https://wiki-power.com/en/PlatformIO%E6%90%AD%E9%85%8DCubeMX%E9%A3%9F%E7%94%A8/#cubemx-initialization-steps
4. https://github.com/ussserrr/stm32pio
5. https://community.platformio.org/t/board-genericstm32h750vb-not-found/36441
