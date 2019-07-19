# Cypress CSDADC Middleware Library 2.0

### What's Included?

Please refer to the [README.md](./README.md) and the [API Reference Guide](https://cypresssemiconductorco.github.io/csdadc/csdadc_api_reference_manual/html/index.html) for a complete description of the CSDADC Middleware.
The revision history of the CSDADC Middleware is also available on the [API Reference Guide Changelog](https://cypresssemiconductorco.github.io/csdadc/csdadc_api_reference_manual/html/index.html#group_csdadc_changelog).
New in this release:
* Updated CSDADC configuration structure to be aligned with the list of parameters in ModusToolbox CSD personality
* After conversion, the ADC channel is still connected to the CSD HW block, and disconnected only prior to new channel connection
* Added the CY_CSDADC_NO_CHANNEL macro
* Added the errata section

### Known Issues
| Problem | Workaround |
| ------- | ---------- |
| GPIO simultaneous operation with unrestricted strength and frequency creates noise that can affect CSDADC operation | Refer to the errata section of the device datasheet for details |

### Defect Fixes
* Fixing a compilation error for non CSDADC-capable devices: CSDADC MW sources are enclosed with the conditional compilation
* Added a cpuClk field to the CSDADC configuration structure
* Changed the Cy_CSDADC_StartConvert() and Cy_CSDADC_InterruptHandler() functions prototype
* Renamed function Cy_CSDADC_ConversionStatus() to Cy_CSDADC_GetConversionStatus()

### Supported Software and Tools
This version of the CSDIDAC Middleware was validated for compatibility with the following Software and Tools:

| Software and Tools                                      | Version |
| :---                                                    | :----:  |
| ModusToolbox Software Environment                       | 2.0     |
| - ModusToolbox Device Configurator                      | 2.0     |
| - ModusToolbox CSD Personality in Device Configurator   | 2.0     |
| PSoC6 Peripheral Driver Library (PDL)                   | 1.2.0   |
| GCC Compiler                                            | 7.2.1   |
| IAR Compiler                                            | 8.32    |
| ARM Compiler 6                                          | 6.11    |
| MBED OS                                                 | 5.13.1  |
| FreeRTOS                                                | 10.0.1  |

### More information
The following resources contain more information:
* [CSDADC Middleware RELEASE.md](./RELEASE.md)
* [CSDADC Middleware API Reference Guide](https://cypresssemiconductorco.github.io/csdadc/csdadc_api_reference_manual/html/index.html)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [CSDADC Middleware Code Example for MBED OS](https://github.com/cypresssemiconductorco)
* [CSDADC Middleware Code Examples at GITHUB](https://github.com/cypresssemiconductorco)
* [ModusToolbox Device Configurator Tool Guide](https://www.cypress.com/ModusToolboxDeviceConfig)
* [CapSense Middleware API Reference Guide](https://cypresssemiconductorco.github.io/capsense/capsense_api_reference_manual/html/index.html)
* [CSDIDAC Middleware API Reference Guide](https://cypresssemiconductorco.github.io/csdidac/csdidac_api_reference_manual/html/index.html)
* [PSoC 6 Technical Reference Manual](https://www.cypress.com/documentation/technical-reference-manuals/psoc-6-mcu-psoc-63-ble-architecture-technical-reference)
* [PSoC 63 with BLE Datasheet Programmable System-on-Chip datasheet](http://www.cypress.com/ds218787)
* [Cypress Semiconductor](http://www.cypress.com)
  
---
© Cypress Semiconductor Corporation, 2019.
