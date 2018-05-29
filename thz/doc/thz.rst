THz Module Documentation
----------------------------

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

THz system module is a software architecture for simulating configurable Terahertz(THz)-band (0.1-10 THz) communication networks. This module takes into account both the nanoscale scenario (transmission distance less than one meter) and the macroscale scenario (transmission distance larger than several meters) of THz-band communication. The THz module consists of the protocol stack layer design from THzChannel to THzPhy(Nano/Macro) up until the THzMac(Nano/Macro). It also includes the supporting modules such as THzDirectionalAntenna and THzEnergyHarvestor to achieve the scenario specific functionalities. Other supporting modules that help modeling the THz channel peculiarities, such as THzSpectrumValueFactory and THzSpectrumPropagationLoss, are also included in the system module. The THz system module provides a comprehensive THz-band communication networks simulator that is expected to help the research community to test THz networking protocols without having to delve into the channel and physical layers. 

In this module, it tests the link-layer transmission performance of the nanoscale scenario with an adhoc network architecture and the macroscale scenario with a centralized network architecture. Both 0-way (ALOHA) handshake and 2-way (CSMA) handshake protocols are implemeted for each scenario. 


Model Description
*****************

The source code for the new module lives in the directory ``src/thz``.

* The frequency database file (data_frequency.txt) and the corresponding molecular absorption coefficient database file (data_AbsCoe.txt) are located inside ``src/thz``.

Design
======

* THzNetDevice: derived from the ns-3 NetDevice class and used for creating new MAC protocols. It performs as the joint point which connects the THzChannel module, THzPhy module and the assistant modules.
* THzChannal: provides a general THz band channel that can be used by any upper layer design.
* THzSpectrumValueFactory: is derived from ns-3 SpectrumModel class, it creates a frequency dependent THz-band based on the HITRAN (HIgh resolution TRANsmission molecular absorption) database and masks the transmit power to user defined bandwidth.
* THzSpectrumPropagationLoss: Creates the frequency and transmission distance dependent propagation loss module based on the peculiarities of THz-band communication.
* THzPhyNano: models the hundred-femto-second pulse based physical layer with pulse interleaving and calculates the SINR (Signal to Noise plus Interference Ratio).
* THzMacNano: models slightly modified version of two classical MAC layer protocol tailored to nanodevice energy harvesting.
* THzEnergyModel: models the energy harvesting and energy consumption process of a node in nanonetworks.
* THzPhyMacro: mainly considers the time duration of a frame being propagated in the THz channel and check if the receiver is able to receive the signal with enough power strength by comparing with the SINR threshold.
* THzMacMacro: implements the 0-way handshake and 2-way handshake protocols, a NAV mechanism is applied in this module.
* THzDirectionalAntenna: is derived from ns-3 CosineAntennaModule class. The main extention in THzDirectional Antenna is enabling the turning ability.



Scope and Limitations
=====================

* In its current state, the THz system module is adapt to the latest ns-3 version (ns-3.28)
* As of now, the ns-3 interface to THz is Ipv4 only.

References
==========

* Z.Hossain, Q.Xia, and J.M.Jornet, "TeraSim: An ns-3 Extension to Simulate Terahertz-band Communication Networks", submitted for journal publication.


Usage
*****

Building THz Module
===================
The first step is to clone THz from the github repository and build it::

 $ git clone https://github.com/UBnano-Terasim/Terasim-ns3
 $ cd Terasim-ns3

Copy the thz folder to your local source folder of ns-3 directory (../ns-allinone-3.28/ns-3.28/src), then go back to ns-allinone-3.28 folder to build the THz module by::

 $ cd ../.. 
 $ ./build.py
 
Once THz has been built successfully, try to run one of the examples (i.e., nano-adhoc.cc). First, you need to copy this example from ns-3.28/src/thz/examples to ns-3.28/scratch::

 $ cp ns-3.28/src/thz/examples/nano-adhoc.cc ns-3.28/scratch/nano-adhoc.cc

Then enter the ns-3.28 folder and run the example::

 $ cd ns-3.28
 $ ./waf --run scratch/nano-adhoc


Helpers
=======
All the helper files can be found in ``src/thz/helper/``:

* THzHelper: helps to create THzNetDevice objects:
* THzMacHelper: create THz MAC layers for THzNetDevice
* THzPhyHelper: create THz PHY layers for THzNetDevice
* THzDirAntennaHelper: create THz directional antenna implementation for THzNetDevice
* THzEnergyModelHelper: installs THzEnergyModel to the nodes.

Attributes
==========

Basically every THz-class in THz module holds attributs. Some key attributes from different classes are summarized as follows:

* THzNetDevice: 

  * Channel: The channel attached to this device
  * DirAntenna: The Directional Antenna attached to this device
  * Phy: The PHY layer attached to this device
  * Mac: The MAC layer attached to this device
* THzChannal: 

  * NoiseFloor: Noise Floor (dBm) 

* THzSpectrumValueFactory: 

  * NumSubBand: The number of sub-bands containing in the selected 3dB frequency window
  * SubBandWidth: The bandwidth of each sub-band
  * TotalBandWidth: The total bandwidth of the selected 3dB frequency window
  * CentralFrequency: The central frequency of the selected 3dB frequency window
  * NumSample: The number of sample bands of the selected 3dB frequency window 
* THzPhyNano: 

  * SinrTh: SINR Threshold (dB)
  * TxPower: Transmission Power (dBm)
  * PulseDuration: Duration of a short pulse
  * Beta: Ratio of symbol duratio to pulse duration
* THzMacNano: 

  * EnableRts: If true, RTS is enabled
  * DataRetryLimit: Maximum Limit for Data Retransmission

* THzEnergyModel:
  
  * EnergyHarvestingAmount: Amount of Energy Harvested in each time
  * PeriodicEnergyUpdateInterval: Time between two consecutive periodic energy updates
* THzPhyMacro: 

  * SinrTh: SINR Threshold (dB)
  * TxPower: Transmission Power (dBm)
  * BasicRate: Transmission Rate (bps) for Control Packets
  * DataRate: Transmission Rate (bps) for Data Packets
* THzMacMacro: 

  * EnableRts: If true, RTS is enabled
  * DataRetryLimit: Maximum Limit for Data Retransmission
* THzDirectionalAntenna: 

  * TuneRxTxMode: If 0, device is a Directional Transmitter; 1, Directional Receiver; 2, Omni-directional Tranceiver
  * BeamWidth: The 3dB beamwidth (degrees)
  * MaxGain: The gain (dB) at the antenna boresight (the direction of maximum gain)
  * TurningSpeed: The turning speed of the Rx antenna unit in circles per second 


Output
======

The link layer performance in terms of the throughput and the discarding probability of DATA packets on each node will be output as the result. Besides, the perfermance of each layer in the protocol stack can be enabled by using LogComponentEnable function in the main function i.e.,::

 $ LogComponentEnable("THzChannel", LOG_LEVEL_ALL);

Examples
===============
The following examples have been written, which can be found in ``src/thz/examples/``:

* nano-adhoc.cc: This example file is for the nanoscale scenario of the THz-band communication networks, i.e., with transmission distance below one meter. It outputs the link layer performance mainly in terms of the throughput and the discarding probability  of the DATA packets. In this example, an adhoc network architecture is implemented. User can set network topology in this file. The nodes in the nanonetwork are equipped with the energy module we developed. The basic parameters of the energy model can be set in this file. User can also set the number of samples of the TSOOK pulse within frequency range 0.9-4 THz window in this file. IUser can select one of the teo MAC protocols that include a 0-way and a 2-way handshake protocols.  0-way starts the link layer transmission with a DATA frame and 2-way with an RTS frame. The selection can be done by setting the attribute value of EnableRts in THzMacNano. In the end, the user can also set the generated packet size and the mean value of the packet generation interval in this file.

* macro-central.cc: This example file is for the macroscale scenario of the THz-band communication networks, i.e., with transmission distance larger than several meters. It outputs the link layer performance mainly in terms of the throughput and the discarding probability of the DATA packets. In this example, a centralized network architecture is implemented. User can set network topology in this file. High speed turning directional antenna is applied at the receiver (Servernodes), while all senders (Clientnodes) pointing the beam of directional antennas toward the receiver. The basic parameters of the directional antennas can be set in this file. User can also set the number of sub-bands and the number of sample bands of the selected 3dB frequency window in this file. There are two MAC protocols can be selected by user in this file, that include a 0-way and a 2-way handshake protocols, which starts the link layer transmission with a DATA frame or a RTS frame respectively. The selection can be achieved by setting the attribute value of EnableRts in THzMacMacro. In the end, the user can also set the generated packet size and the mean value of the packet generation interval in this file.

Validation
**********

This model has been tested validated by the results generated from the following test files, which can be found in ``src/thz/test``:

* The test files ``test-thz-psd-macro.cc`` and ``test-thz-psd-nano.cc`` are used to plot the power spectral densities of the generated waveform by the physical layer and the received signal at certain distance for macroscale scenario and nanoscale scenario respectively.
* The test file ``test-directional-antenna.cc`` plots the antenna radiation pattern of the directional antenna.
* The test file ``test-thz-path-loss.cc`` plots the path loss as a function of distance.

Copy Right
**********
http://ubnano.tech/


