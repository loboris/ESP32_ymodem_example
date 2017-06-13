
### Example of using YModem protocol on ESP32

---


#### Features

* Enables **YModem** protocol file transfer over UART
* Send and receive functions included
* **Demo application** included
* Used terminal application must support ymodem file transfer. Tested with **minicom**

---

#### How to build

Configure your esp32 build environment as for **esp-idf examples**

Clone the repository

`git clone https://github.com/loboris/ESP32_ymodem_example.git`

Execute menuconfig and configure your Serial flash config and other settings. Included *sdkconfig.defaults* sets some defaults to be used.

Navigate to **Ymodem example** and set **SPIFFS** options.

Select if you want to use **wifi** (recommended) to get the time from **NTP** server and set your WiFi SSID and password.

`make menuconfig`

Make and flash the example.

`make all && make flash`

---

#### Prepare **SPIFFS** image

*The example uses spiffs file system to receive and send files*

The file system will be formated on first start, unless you flashed prepared spiffs image before the first start.

---

You can prepare SFPIFFS **image** and flash it to ESP32.

Files to be included on spiffs must be placed in **components/spiffs_image/image/** directory. You can add or remove the files you want to include.

Then execute:

`make makefs`

to create **spiffs image** in *build* directory **without flashing** to ESP32

Or execute:

`make flashfs`

to create **spiffs image** in *build* directory and **flash** it to ESP32

---

To flash already prepared image **components/spiffs_image/spiffs_image.img** execute:

`make copyfs`

---

**Example output:**

```

I (1079) cpu_start: Pro cpu up.
I (1091) cpu_start: Starting app cpu, entry point is 0x40080f18
I (0) cpu_start: App cpu up.
I (1124) heap_alloc_caps: Initializing. RAM available for dynamic allocation:
I (1146) heap_alloc_caps: At 3FFAE2A0 len 00001D60 (7 KiB): DRAM
I (1167) heap_alloc_caps: At 3FFB7AD8 len 00028528 (161 KiB): DRAM
I (1188) heap_alloc_caps: At 3FFE0440 len 00003BC0 (14 KiB): D/IRAM
I (1209) heap_alloc_caps: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (1231) heap_alloc_caps: At 40094980 len 0000B680 (45 KiB): IRAM
I (1252) cpu_start: Pro cpu start user code
I (1306) cpu_start: Starting scheduler on PRO CPU.
I (197) cpu_start: Starting scheduler on APP CPU.
I (197) uart: queue free spaces: 10
I (197) [Ymodem example]: Time is not set yet. Connecting to WiFi and getting time over NTP.
I (237) wifi: wifi firmware version: 9bf11be
I (237) wifi: config NVS flash: enabled
I (237) wifi: config nano formating: disabled
I (237) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE
I (247) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE
I (277) wifi: Init dynamic tx buffer num: 32
I (277) wifi: Init dynamic rx buffer num: 32
I (277) wifi: wifi driver task: 3ffc3194, prio:23, stack:4096
I (287) wifi: Init static rx buffer num: 10
I (287) wifi: Init dynamic rx buffer num: 32                                                                 
I (287) wifi: Init rx ampdu len mblock:7                                                                     
I (297) wifi: Init lldesc rx ampdu entry mblock:4                                                            
I (297) wifi: wifi power manager task: 0x3ffc855c prio: 21 stack: 2560                                       
I (307) [Ymodem example]: Setting WiFi configuration SSID LoBoInternet...                                    
I (317) wifi: wifi timer task: 3ffc95dc, prio:22, stack:3584                                                 
I (337) phy: phy_version: 355.0, 2888565, May 23 2017, 16:12:06, 1, 0                                        
I (337) wifi: mode : sta (24:0a:c4:00:97:c0)                                                                 
I (467) wifi: n:1 0, o:1 0, ap:255 255, sta:1 0, prof:1                                                      
I (1447) wifi: state: init -> auth (b0)                                                                      
I (1447) wifi: state: auth -> assoc (0)                                                                      
I (1447) wifi: state: assoc -> run (10)                                                                      
I (1487) wifi: connected with LoBoInternet, channel 1                                                        
I (3697) event: ip: 192.168.0.16, mask: 255.255.255.0, gw: 192.168.0.1                                       
I (3697) [Ymodem example]: Initializing SNTP                                                                 
I (4197) [Ymodem example]: System time is set.                                                               
I (4197) wifi: state: run -> init (0)                                                                        
I (4197) wifi: n:1 0, o:1 0, ap:255 255, sta:1 0, prof:1                                                     
E (4197) wifi: esp_wifi_connect 816 wifi not start                                                           
                                                                                                             
                                                                                                             
I (5197) [SPIFFS]: Registering SPIFFS file system                                                            
I (5197) [SPIFFS]: Mounting SPIFFS files system                                                              
I (5197) [SPIFFS]: Start address: 0x180000; Size 1024 KB                                                     
I (5197) [SPIFFS]:   Work buffer: 2048 B                                                                     
I (5207) [SPIFFS]:    FDS buffer: 384 B                                                                      
I (5207) [SPIFFS]:    Cache size: 2048 B                                                                     
I (5267) [SPIFFS]: Mounted                                                                                   
I (5267) [Ymodem example]: File system mounted.                                                              
Removed "/spiffs/yfile-1.bin"                                                                                
Removed "/spiffs/yfile-6.bin"                                                                                
LIST of DIR [/spiffs/]                                                                                       
T  Size      Date/Time         Name                                                                          
-----------------------------------                                                                          
d         -                    /spiffs                                                                       
d         -  12/06/2017 15:09  images                                                                        
d         -  12/06/2017 15:09  fonts                                                                         
f       405  12/06/2017 15:09  spiffs.info                                                                   
-----------------------------------                                                                          
        405 in 1 file(s)                                                                                     
-----------------------------------                                                                          
SPIFFS: free 776 KB of 957 KB                                                                                
                                                                                                             
                                                                                                             

Receiving file, please start YModem transfer on host ...                                                     
CCCCCCCCCCC
I (41427) [Ymodem example]: Transfer complete, Size=97543, orig name: "/spiffs/yfile-1.bin"
LIST of DIR [/spiffs/]
T  Size      Date/Time         Name
-----------------------------------
f     97543  13/06/2017 14:20  yfile-1.bin
-----------------------------------
      97543 in 1 file(s)
-----------------------------------
SPIFFS: free 680 KB of 957 KB

Sending file "/spiffs/yfile-1.bin", please start YModem receive on host ...
CCCCC
I (66667) [Ymodem example]: Transfer complete.

```
