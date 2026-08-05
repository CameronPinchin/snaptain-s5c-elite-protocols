# <p align='center' >Snaptain S5C Elite Reverse Engineering  </p>

# Table of Contents

* Technical Notes
* Introduction
* Tooling Used
* Resources

__NOTE(1)__: If you have any questions or need further clarification, you are welcome to create an issue and ask.

# Technical Notes

__NOTE(2)__: This section is intended to provide a centralized source of technical information regarding the networking architecture for the Snaptain S5C Elite drone. 

## <p align='center'> Networking Overview </p>

The drone initiates a hotspot network approximately 3 seconds after power-on under the name: **SNAPTAIN ELITE S5C A77D3E**. This network is unsecured and requires no password by default, as such anybody within a ~20 meter range of this drone will be able to connect to its network. There is a *minor* security layer added to be able to access the live video feed, but is easily emulated and would not be an issue for anyone who knows what they are doing.

For normal operating conditions, if a device satifies two conditions, they will be able to view the live video feed streaming from the drone:
    (1): An external device must be connected to the drone's network.
    (2): The external device must have the *SNAPTAIN FPV* app installed and opened following a successful initial connection.

For a dedicated individual, they would only require the first condition to be satisfied to gain access to the live video feed (e.g., they need to be in close proximity to the drone in order to access its network).

This repository intends to document the steps I took to reverse engineer the networking protocols for this drone, and provide a specifications sheet as a quick reference guide if anyone requires this information.

## *<p align='center'> Networking Overview | Initial Handshake </p>*

The initial handshake describes the environment after the drone has been powered on, but no devices have connected to it yet.

