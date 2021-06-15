TeraSim
------------------------------------------------
TeraSim is the first simulation platform for THz communication networks which captures the capabilities of THz devices and the peculiarities of the THz channel. The simulator has been developed considering two major types of application scenarios, namely, nanoscale communication networks (average transmission range usually below one meter) and macroscale communication networks (distances larger than one meter). The simulator consists of a common channel module, separate physical and link layers for each scenario, and two assisting modules, namely, THz antenna module and energy harvesting module, originally designed for the macroscale and nanoscale scenario, respectively. TeraSim is expected to enable the networking community to test THz networking protocols without having to delve into the channel and physical layers. 

A detailed explanation of the module can be found in [TeraSim: An ns-3 extension to simulate Terahertz-band communication networks](https://doi.org/10.1016/j.nancom.2018.08.001).


v1.1 patch notes
------------------------------------------------
The latest TeraSim v1.1 release includes the following improvements:
- Implementation of [ADAPT: An Adaptive Directional Antenna Protocol for medium access control in Terahertz communication networks](https://doi.org/10.1016/j.adhoc.2021.102540). This is a MAC protocol for the macroscale scenario.
- Optimization of the code for improved computational efficiency


Model Description, Usage and Validation of the TeraSim
------------------------------------------------
Please refer to the thz.rst in the thz/doc folder


Copy Right
------------------------------------------------
Northeastern University https://unlab.tech/
