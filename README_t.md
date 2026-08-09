# <p align='center' >Snaptain S5C Elite Reverse Engineering  </p>

# Table of Contents

* Technical Notes
* Networking Overview
* Native Library Reverse Engineering
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
| **AES Key**               | `guanxukj@fh8620.`            | AES-128 encryption key used to encrypt transmission data.
| **Libraries**             | `libFHDEV_NET.so`             | Shared object file identified in the apps source code, specific to the SoC.

The libFHDEV_NET.so shared-object file was decompiled using [Ghidra](https://www.nsa.gov/ghidra) and provided the packet architecture the drone expects to receive. The pseudocode for critical functions can be found in the **pseudocode** directory in this repository, with the primary functions of interest being: FHDEV_NET_Login, DM_Login, NC, and TCPSocketSend.  

The packets are sent with an 81-byte header with a 1-byte payload, bringing the packet size to 82 bytes. The structure identified in the NC function decompiled in Ghidra looks like so:  

| Parameter                             | Value                             | Notes
| :---                                  | :---                  | :---
| **undefined1 local_10c0**             | `device_type`         | This is set to 0x00.
| **byte local_10bf**                   | `g_ucHeadLen`         | This is the header length. Constant; set to 0x51 == 81.
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

For a cleaner overview of the packet architecture, it can be seen below in the same ordering:

```
Offset  0  (1 byte)  : device_type     - 0x00 for model_id=0xff
Offset  1  (1 byte)  : g_ucHeadLen     - 0x51 = 81
Offset  2  (1 byte)  : a9              - 0x00
Offset  3  (1 byte)  : cmd_id          - 0x01
Offset  4  (1 byte)  : seq_id          - 0x01
Offset  5  (4 bytes) : error_code      - 0x00000000
Offset  9  (1 byte)  : padding         - 0x00
Offset 10  (32 bytes): username        - "guanxukeji\0" + zeros
Offset 42  (36 bytes): password        - "gxrdw60\0" + zeros
Offset 78  (1 byte)  : flag            - 0x00
Offset 79  (2 bytes) : payload_len+1   - 0x0001 (LE)
---------------------------------------------------------------- [End of 81-byte header]
Offset 81  (1 byte)  : a10             - 0x00
Offset 82  (+)       : payload         - 1 zero byte for login
```

### Primary Handshake | Login Packet, AES Encryption

__NOTE(4)__: Encrpytion in general is an area I am unfamiliar with. If anything in this section is lacking, please raise an issue and I will review it.

The psuedocode obtained from the libFHDEV_NET.so file revealed an AES Encryption step for the packets. The function AESSocketSend identified the type of AES encryption as **AES-128-ECB** simply due to the encryption loop visible in the function:  
```
    do {
        aes_enc_blk(plaintext + offset, ciphertext + offset, key);
        offset += 16;
    } while (offset < plaintext_len) 
```

The AES encryption method requires the input to be a multiple of 16 bytes. The plaintext for our packet is 82 bytes; so the nearest 16-byte multiple is 96 which would require 14-bytes of zero-padding, making our unencrypted plaintext 96 bytes (82 bytes of data, 14 bytes of padding). The AESSocketSend function encrypts this plaintext, and then wraps the ciphertext in a 10-byte framing header. The framing header layout can be seen below:  
```
Bytes 0,1   : 0x49,0x54     - ASCII "IT" magic number
Bytes 2,5   : iVar3         - LE int32, value = (last_block_offset + 20)
Bytes 6,9   : param_3       - LE int32, original plaintext length
Bytes 10,n  : ciphertext    - The 96-byte AES-ECB encrypted data.
```
In total, a 106 byte packet is sent following the AES encryption layer and framing header. This command structure has been verified and an example exchange can be seen below, with the source code for the tool using found in the *tooling* directory.

```
[SUCCESS] Successfully connected to 172.19.10.0:8866
Wire packet (106 bytes):
49 54 64 00 00 00 53 00 00 00 bb b2 99 39 25 b2 
a4 c3 dc 01 d8 b1 b5 11 5b 98 92 db 3e 6a fc 10 
50 2d 79 80 0c a1 a5 e5 ba d4 aa 2d 95 15 81 b4 
ab 82 2f 3f db d0 07 38 a6 2f 8a 31 44 a7 32 2c 
11 dc 24 5d e0 17 f9 14 4c cc 9a a3 5b 13 14 7e 
2e 76 f8 1e 22 c1 70 5b b1 27 a5 0b 32 60 d3 c0 
f8 00 db d5 b7 7b 51 de 89 13 

Login packet sent

Response (106 bytes):
49 54 64 00 00 00 53 00 00 00 b6 73 09 27 1c f5 
ce 88 fc 78 71 39 69 f3 97 7d a5 0b 32 60 d3 c0 
f8 00 db d5 b7 7b 51 de 89 13 a5 0b 32 60 d3 c0 
f8 00 db d5 b7 7b 51 de 89 13 a5 0b 32 60 d3 c0 
f8 00 db d5 b7 7b 51 de 89 13 36 ab 51 9f 82 4a 
9c b3 44 a8 67 ef 57 37 be 8f 49 87 07 2b 74 82 
e5 92 41 b1 f0 2d 5f fa 3b b5 

Decrypted response (83 bytes):
00 00 00 01 02 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 
00 00 09 

Device type byte:       0x00
Response cmd_id:        0x01
Response seq_id:        0x02
Response error_code:    0x0000
Payload byte:           0x09
```

(ADD MORE HERE!!)

Protocol diagram for the login process:

```
                    S5C Elite
                       │
                 Wi-Fi AP starts
                       │
                       ▼
              ┌─────────────────┐
              │  UDP multicast  │
              │    discovery    │
              └────────┬────────┘
                       │
                 TCP: 8866
                       │
                       ▼
              ┌─────────────────┐
              │ Login plaintext │
              │   81 + 1 bytes  │
              └────────┬────────┘
                       │
                       ▼
              ┌─────────────────┐
              │   AES-128-ECB   │
              │   + "IT" frame  │
              └────────┬────────┘
                       │
                       ▼
              ┌─────────────────┐
              │ Login response  │
              └─────────────────┘
```









