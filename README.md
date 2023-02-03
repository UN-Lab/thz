# TeraSim - An ns-3 Module for THz Networks #

This is an [ns-3](https://www.nsnam.org "ns-3 webpage") module for the simulation of Terahertz(THz)-band (0.1-10 THz) communication networks.

This module is the first simulation platform for which captures the capabilities of THz devices and the peculiarities of the THz channel. The simulator has been developed considering two major types of application scenarios, namely, nanoscale communication networks (average transmission range usually below one meter) and macroscale communication networks (distances larger than one meter). The simulator consists of a common channel module, separate physical and link layers for each scenario, and two assisting modules, namely, THz antenna module and energy harvesting module, originally designed for the macroscale and nanoscale scenario, respectively. TeraSim is expected to enable the networking community to test THz networking protocols without having to delve into the channel and physical layers.

A detailed explanation of the module can be found in
[this paper](https://doi.org/10.1016/j.nancom.2018.08.001).

## Installation ##
The first step is to clone the module from the github repository to your local source folder of ns-3 directory `ns-3.37/contrib`:
```bash
git clone https://github.com/UN-Lab/thz.git
```
Then go back to ns-3.37 folder to build the THz module by:
```bash
cd ..
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

## Usage example ##
You can use the following command to run the `thz-macro-central` example.
```
./ns3 run thz-macro-central
```
Other examples are included in `examples`.

## Documentation ##
The documentation of this module is available at [this link](./doc/source/thz.rst).

## References ##
The following papers describe in detail the features implemented in the TeraSim module:
- [TeraSim: An ns-3 extension to simulate Terahertz-band communication networks](https://doi.org/10.1016/j.nancom.2018.08.001) describes the whole module. We advise the researchers interested in this module to start reading from this paper.
- [ADAPT: An Adaptive Directional Antenna Protocol for medium access control in Terahertz communication networks](https://doi.org/10.1016/j.adhoc.2021.102540) describes the implementation of a new MAC protocol for the macroscale scenario.

If you use this module in your research, please cite: Z. Hossain, Q. Xia, J. M. Jornet, _"TeraSim: An ns-3 extension to simulate Terahertz-band communication networks"_ in Nano Communication Networks, 2018.

## About ##
This module is being developed by [UN Laboratory](http://https://unlab.tech/), [Northeastern University](https://www.northeastern.edu/).

## Authors ##
- Zahed Hossain, University at Buffalo
- Qing Xia, University at Buffalo
- Daniel Morales, Northeastern University
- Josep Miquel Jornet, Northeastern University

## License ##
This software is licensed under the terms of the GNU GPLv2, as like as ns-3. See the LICENSE file for more details.
