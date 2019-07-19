# Cypress CSDADC Middleware Library

### Overview
The CSDADC middleware is the Cypress ADC solution that uses CSD HW block for measurements. It is useful for a devices that do not include another ADC option. The CSDADC provides the following measurement capabilities:
* Voltage monitoring on multiple external channels.
* Voltage monitoring on AMUX-B.
* Device supply voltage (VDDA) monitoring without need of explicitly connecting VDDA to a GPIO input of ADC. This capability can be used to measure battery voltage and/or change VDDA dependent parameters of the ADC during run-time.

The listed capabilities are making the CSDADC useful for variety of applications, including home appliances, automotive, IoT, and industrial applications. The CSDADC middleware can use the same CSD HW block with other CSD based middlewares (CapSense, CSDIDAC, etc) in time-multiplexed manner.

### Features
* ADC with 8- and 10-bit resolution
* Two input measurement ranges: GND to VREF and GND to VDDA
* Two operation modes: Continuous conversion and single shot conversion

### Quick Start

The CSDADC could be configured by the ModusToolbox CSD personality. Refer to the [API Reference Guide Configuration Considerations](https://cypresssemiconductorco.github.io/csdadc/csdadc_api_reference_manual/html/index.html#group_csdadc_configuration).


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
