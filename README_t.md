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

The drone initiates a hotspot network approximately 3 seconds after power-on under the name: **SNAPTAIN ELITE S5C A7D33E**. This network is unsecured and requires no password by default, as such anybody within a ~20 meter range of this drone will be able to connect to its network. There is a *minor* security layer added to be able to access the live video feed, but is easily emulated and would not be an issue for anyone who knows what they are doing.

For normal operating conditions, if a device satifies two conditions, they will be able to view the live video feed streaming from the drone:  
```
    (1): An external device must be connected to the drone's network.  
    (2): The external device must have the *SNAPTAIN FPV* app installed and opened following a successful initial connection.
```
For a dedicated individual, they would only require the first condition to be satisfied to gain access to the live video feed (e.g., they need to be in close proximity to the drone in order to access its network).

This repository intends to document the steps I took to reverse engineer the networking protocols for this drone, and provide a specifications sheet as a quick reference guide if anyone requires this information.

## *<p align='center'> Networking Overview | Initial Handshake </p>*

The initial handshake describes the environment after the drone has been powered on, but no devices have connected to it yet.

Architecture of Initial Handshake:
```
1. The drone acting as an Access Point (AP) begins transmitting beacon frames on radio frequency (RF) 2.4 GHz band on channel 2.

2. The device transmits a probe request to identify the network; the drone responds with a probe response.

3. The device transmits an authentication request to the network; the drone responds with an authentication response. 

4. The device transmits an association request to the network; the drone responds with an association response. 
```

The network does not require a password by default. From preliminary research, it appears you also cannot modify this yourself to improve security. This means, after the association exchange (4) is complete, the device has connected to the network. 

At this stage, any data being transmitted to and from the network is unencrypted and broadcasted in plaintext. This data can be intercepted by individuals in close proximity and the means to do so.

### Initial Handshake | Key Network Information 

The information regarding key network information required at this stage can be found in the table below:

| Parameter     | Value                         | Notes
| :---          | :---                          | :---
| **SSID**      | `SNAPTAIN ELITE S5C A7D33E`   | This should be consistent across all models.
| **Password**  | `NULL`                        | No password; you cannot set one either.
| **RF Band**   | `2.4 GHz`                     | IEEE 802.11b/g/n
| **Channel**   | `2`                           | Operates at 2417 MHz
| **Static IP** | `172.19.10.1`                 | Identified by decompiling the SNAPTAIN FPV app.

__NOTE(3)__: I have not observed the drone operating on a different channel other than 2, but have not verified that this is hardcoded. I will provide an amendment following further research.

## *<p align='center'> Networking Overview | Primary Handshake </p>*

Following a successful initial handshake, the device has established a successful connection to the drone's network. The 'primary' handshake regards establishing a connection to the live video streaming from the drone. It is only considered 'primary' for the purposes of differentiating the initial network connection and establishing a live video connection.

This handshake has an added layer of security, but that layer is rather thin and took me a few hours to bypass it.  

For an average users, the process for establishing a live video feed from the drone is as follows:
```
1. From the users phone, connect to the drone's WiFi network.
2. Start the SNAPTAIN FPV app after a successful connection.
3. Select the 'FUNCTION' button in the bottom-right corner and a live video stream appears.
```

### Primary Handshake | Technical Overview

To emulate this process, I connected a PC to the network and leveraged *tshark* to reveal information about communications between the devices. This provided additional key information:  

1. The drone transmits 234 byte UDP packets to a multicast group with the destination port 51167 on a two second interval. 

2. These packets are transmitted without any identifiable prompting event, and can be captured by any device capable of joining that multicast group. 

3. The packets are unencrypted.  

The packets contain useful information for decoding the networking process. Here is a sample packet collected:  
```
f0 bf 00 00 01 00 01 00 01 00 01 00 02 00 09 00  
00 00 d4 00 00 00 35 30 2d 39 42 2d 39 34 2d 41  
37 2d 44 33 2d 33 45 00 31 37 32 2e 31 39 2e 31  
30 2e 31 00 00 00 00 00 32 35 35 2e 32 35 35 2e  
30 2e 30 00 00 00 00 00 31 37 32 2e 31 39 2e 31  
30 2e 31 00 00 00 00 00 a2 22 30 2e 30 2e 30 20  
28 62 75 69 6c 64 20 30 29 00 00 00 00 00 00 00  
00 00 00 00 00 00 00 00 00 00 30 2e 30 2e 30 20  
28 62 75 69 6c 64 20 30 29 00 00 00 00 00 00 00  
00 00 00 00 00 00 00 00 00 00 46 48 38 38 33 30  
5f 35 30 2d 39 42 2d 39 34 2d 41 37 2d 44 33 2d  
33 45 00 00 00 00 00 00 00 00 35 30 2d 39 42 2d  
39 34 2d 41 37 2d 44 33 2d 33 45 00 00 00 00 00  
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  
00 00 00 00 00 00 00 00 00 00 
```

The packets contain more than will be listed below, but most of it is redunant and was covered earlier in this document. This includes: drone IP address, gateway address, and subnet mask.

| Parameter                 | Value                         | Notes
| :---                      | :---                          | :---
| **MAC Address**           | `50:9B:94:A7:D3:3E`           | --
| **Firmware Version**      | `0.0.0 (build 0)`             | -- 
| **Hardware Information**  | `FH8830_50-9B-94-A7-D3-3E`    | The SoC for the camera on the drone + MAC Address.
| **TCP Port**              | `8866`                        | An open TCP port used by the network.

The drone does not transmit data through this port unprompted. I could identify no data transmission to or from this port during initial tshark scans, nor could I capture any packets when listening on that specific port. This indicates the drone expects a prompting event, which marks the first stage of the primary handshake

### Primary Handshake | Login Packet

This was the extent of the useful information gathered from looking at the data exchanges between the drone and a connected device. The next step involved downloading the Android version of the SNAPTAIN FPV app and looking through the source code. Using some of the information found earlier (the TCP port, in particular), identified critical information used in the authentication process: 

| Parameter                 | Value                         | Notes
| :---                      | :---                          | :---
| **LOGIN_TWICE**           | `false`                       | Boolean value. Set to false, doesn't seem to change.
| **userName**              | `guanxukeji`                  | Primary username used in the authentication process. 
| **password**              | `gxrdw60`                     | Primary password used in the authentication process.
| **userName2**             | `guanxukeji2`                 | Optional username; required if LOGIN_TWICE is true. 
| **password2**             | `gxrdw602`                    | Optional password; required if LOGIN_TWICE is true. 
| **AES Key**               | `guanxukj@fh8620.`            | AES-128 encryption key used to encrypt video transmission data.
| **Libraries**             | `libFHDEV_NET.so`             | Shared object file identified in the apps source code, specific to the SoC.

The libFHDEV_NET.so shared-object file was decompiled using [Ghidra](https://www.nsa.gov/ghidra) and provided the packet architecture the drone expects to receive. The pseudocode for critical functions can be found in the **pseudocode** directory in this repository, with the primary functions of interest being: FHDEV_NET_Login, DM_Login, NC, and TCPSocketSend.  

The packets are sent with an 82-byte header with a 1-byte payload, bringing the packet size to 83 bytes. The structure identified in the NC function decompiled in Ghidra looks like so:  

| Parameter                             | Value                             | Notes
| :---                                  | :---                  | :---
| **undefined1 local_10c0**             | `device_type`         | This is set to 0x00.
| **byte local_10bf**                   | `g_ucHeadLen`         | This is the header length. Constant; set to 0x52 == 82.
| **undefined1 local_10be**             | `param_9 (a9)`        | Passed as 0. Purpose is unclear.
| **char local_10bd**                   | `cmd_id`              | The command being issued. 0x01 == login command. 
| **byte local_10bc**                   | `seq_id`              | Sequence ID for the packets. 0x01 is the first packet, the response is seq_id+1.
| **byte local_10bb**                   | `error_code`          | Zero filled with a memset call. Holds error codes.
| **char acStack_10b6[32]**             | `username`            | strcpy'd from param_5 = "guanxukeji".
| **char acStack_1096[36]**             | `password`            | strcpy'd from param_6 = "gxrdw60".
| **undefined1 local_1072**             | `extra_flag`          | Function unknown, but always 0 if not null.
| **ushort     local_1071**             | `length`              | Set to payload_length + 1; for login this is 1.
| **undefined1 local_106f**             | `param_10 (a10)`      | Passed as 0 for login.
| **undefined1 auStack_106e[4102]**     | `payload buffer`      | The buffer for the actual payload data.













