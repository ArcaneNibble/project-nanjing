# Reverse engineering the WCH BLE hardware

This project is an attempt to reverse engineer the BLE hardware on WCH microcontrollers, currently targeting the CH579. The goal would be to replace this with a fully open BLE stack without blobs.

# Current status

It is possible to manually transmit and receive single BLE packets.

# What do I need to hack on this?

A devkit can be acquired from [AliExpress](https://www.aliexpress.com/item/1005005463788950.html) for around ten currency units (USD/EUR/GBP). Make sure to get a CH579**M** and not a CH579**F**, as only the CH579M has a SWD interface. The CH579M-EVT-R1 with Ethernet is recommended.

The official SDK can be downloaded from [here](https://www.wch.cn/downloads/CH579EVT_ZIP.html). Click the very large "下载" (download) button. The English-language datasheet can be found [here](https://www.wch-ic.com/downloads/CH579DS1_PDF.html).

# Poking at the SDK

If you get garbage characters (mojibake) when opening SDK files, tell your editor to use GB18030/GB2312 legacy Chinese character sets. Files are not UTF-8.

After unpacking the .zip, the peripheral driver code is in the `EVT/EXAM/SRC/StdPeriphDriver` directory, the BLE blob is in `EVT/EXAM/BLE/LIB`, and non-blob BLE-associated code is in `EVT/EXAM/BLE/HAL`.

The official SDK is set up for Keil. If you want to use GCC, you will need to manually duct-tape together the usual Cortex-M startup vector table and linker script stuff. There is an example here.

Linking the BLE `.lib` will not work for some reason, but using the `.hex` will (with the caveat of course that the `.hex` is much less flexible with memory layout).

The official SDK requires a `__nop` function and might have other incompatibilities that are not immediately blocking for hacking on BLE (`warning: 'packed' attribute ignored`). This probably needs to be fixed for a production-grade FOSS ecosystem.

The official SDK invokes a `SystemInit` function (in `CH57x_clk.c`) before calling `main`. This function pokes undocumented registers which (wild guessing) probably configure flash wait states.

The BLE blob is built on a task scheduler system called TMOS (which is *not* a preemptive RTOS). There are some notes about how to use it [here](https://codeberg.org/20-100/Awesome_RISC-V/src/branch/master/WCH/WCH_TMOS_HowTo.pdf).

# Hardware blocks

The datasheet documents the following hardware blocks relating to the radio:

* DMA @ 0x4000C000
* BB @ 0x4000C100
* LLE @ 0x4000C200
* AES @ 0x4000C300

There is an completely unmentioned and undocumented block, which symbols in the blob call RFEND:

* RFEND @ 0x4000D000

The blocks seem to more-or-less control the following:

* DMA -- Direct Memory Access. Gets and puts packet data from system memory.
* BB -- BaseBand?. The documented registers control CRCs and access addresses, but it isn't clear what else this can do.
* LLE -- Link Layer / Low Level (E?). This seems to control sequencing between transmitting and receiving.
* AES -- cryptography
* RFEND -- analog PHY parameters

## DMA

The registers here are mirrored every 0x20 bytes

* +0x00
    * no accesses seen from the blob
    * bit7 can be set, nothing else can be set
* +0x04
    * no accesses seen from the blob
    * nothing can be set
* +0x08 R32_DMA0_CTRL_CFG
* +0x0c R32_DMA2_CTRL_CFG
    * bits [12:0] are set to a length when transmitting
    * bit 13 is set on init, might be the "IRQMASK" described by the datasheet
    * bits[15:14] can also be set, unknown function
    * bits[31:16] cannot be set
* +0x10 R32_DMA0_TX_SRC
* +0x14 R32_DMA2_TX_SRC
* +0x18 R32_DMA0_RX_DST
* +0x1c R32_DMA2_RX_DST
    * ram address of a buffer

## BB

The registers here are mirrored every 0x80 bytes

* +0x00 R32_BB_CTRL_CFG
    * bits [5:0] set the channel (BLE channel numbering)
    * bit 6 *might* disable whitening (not fully tested)
    * can set bits 0x7f7fffff
* +0x04 R32_BB_TXCRC_INIT
* +0x08 R32_BB_TXACCS_ADDR
* +0x0c R32_BB_RXCRC_INIT
* +0x10 R32_BB_RXACCS_ADDR
* +0x14
    * reset value 0x45514ab3
	* can set 0xffffffff
* +0x18
    * reset value 0xc60a663f
	* can set 0xffffffff
* +0x1c
    * reset value 0x00140000
	* can set 0x003fffff
* +0x20
    * reset value 0x00205830
	* can set 0x00ffffff
* +0x24
    * reset value 0x810e026c
	* can set 0xffffffff
* +0x28
    * reset value 0x000908be
	* can set 0x3fffffbe
	* cannot clear the 0xbe bits
	* 0xbe bits somehow affected by above unk registers
* +0x2c R32_BB_CTRL_TX
    * can set 0x71fffffb
    * poked by blob a bunch
* +0x30 R32_BB_RSSI_ST
    * bits [19:12] might be signed rssi byte?
    * other bits change too
    * might be R/O
* +0x34 R32_BB_INT_EN
	* can set 0x7f
* +0x38 R32_BB_INT_ST
	* R/O?
* +0x3C
    * seems to mirror R32_BB_TXACCS_ADDR
* +0x40 .. 0x54
    * always zero?
* +0x58 .. 0x7c
    * seems to mirror R32_BB_TXACCS_ADDR

Is there vestigial 802.15.4 support anywhere? There was supposed to be originally, but the feature was canceled.

## LLE

* +0x00 R32_LLE_CTRL_CMD
    * write 1 = CMD_RX
    * write 2 = CMD_TX
    * write 4 = CMD_STOP?
    * write 8 = CMD_SHUT?
* +0x04 R32_LLE_CTRL_CFG
	* reset to 0xf01
	* can set 0xff1f
	* bit 0 = auto mode
* +0x08 R32_LLE_STATUS
	* bit4 = happens when CMD_SHUT
	* RX -> 0x5
	* TX -> 0xa
	* bit20 happens when RX or TX but R32_LLE_CTRL_MOD isn't set
	* bit21 happens when TX but R32_LLE_CTRL_MOD isn't set
* +0x0c R32_LLE_INT_EN
	* reset to 0x3f1f
	* can *only* set 0x3f1f
* +0x10 _R32_THINGY0
* +0x14 _R32_TIMING0
* +0x18 _R32_THINGY1
* +0x1c _R32_TIMING1
* +0x20 _R32_THINGY2
* +0x24 _R32_TIMING2
* +0x28 _R32_THINGY3
* +0x2c _R32_TIMING3
* +0x30 _R32_THINGY4
* +0x34 _R32_TIMING4
* +0x38 _R32_THINGY5
* +0x3c _R32_TIMING5
* +0x40 _R32_THINGY6
* +0x44 _R32_TIMING6
* +0x48 _R32_THINGY7
* +0x4c _R32_TIMING7
* +0x50 R32_LLE_CTRL_MOD
	* 0x58	??? mentioned in datasheet
	* 0x59	rx
	* 0x5a	tx
	* 0x5d	tuning???
	* 0x79	pkt dec
	* 0x7a	pkt enc
    * note that RX/TX of a single packet seems to work just setting 0x58
    * setting bit 0x40 automatically sets bit 0x08
	* bit 3 = BB or LLE ???
	* bit 4 = enable DMA properly? (if not set, writes one word repeatedly, doesn't transmit)
	* bit 5 = enable AES???
	* bit 6 = BB or LLE ???
* +0x54 _R32_THINGY8
    * for each thingy register, can set bit 0x20
* +0x58 _R32_TIMING8
    * for each timing register, can set all bits
    * blob calculates values for this
* +0x5c .. 0xfc
    * always zero?

The datasheet mentions "5" independent hardware timers, but it seems to be 4x2 and one additional one?

## AES

XXX weird mirroring happens

* +0x00 R32_AES_CTRL_CCMMOD
	* bit 6 = ?????
	* bit 5 = directionBit ?????
	* bit [4:3]
		* 00 = aes128
		* 01 = aes192
		* 10 = aes256
		* 11 = aes128 again?
	* bit [2:1]
		* 00 = encrypt
		* 01 = decrypt
		* 10 = rand ???
		* 11 = decrypt again?
	* bit 0 = go
* +0x04 R32_AES_CCMINT_EN
	* bit 1 = current operation done???
	* bit 0 = enable interrupt / status flag???
* +0x08 R32_AES_CCMVT_INIT0
* +0x0c R32_AES_CCMVT_INIT0
* +0x10 R32_AES_PKT_CNT0
* +0x14 R32_AES_PKT_CNT1
    * only 7 bits in this register
* +0x18 R32_AES_DATA0
* +0x1c R32_AES_DATA1
* +0x20 R32_AES_DATA2
* +0x24 R32_AES_DATA3
* +0x28 R32_AES_KEY0
* +0x2c R32_AES_KEY1
* +0x30 R32_AES_KEY2
* +0x34 R32_AES_KEY3
* +0x38 R32_AES_KEY4
* +0x3c R32_AES_KEY5
* +0x40 R32_AES_KEY6
* +0x44 R32_AES_KEY7
    * key4-key7 can be written but cannot be read
* +0x48 R32_AES_RAND0
* +0x4c R32_AES_RAND1
* +0x50 R32_AES_RAND2
* +0x54 R32_AES_RAND3
    * can be written
    * don't understand this
* +0x58 .. 0x7c
    * always zero?
* +0x80 _R32_AES_ERROR_AND_CCM_XXX
    * writing some values to this causes a computation and wedges the unit
    * blob does some kind of error checking here
* +0x84 
* +0x88 
* +0x8c 
* +0x90 
* +0x94 
* +0x98 
* +0x9c 
* +0xa0 
    * some of these are writable, ???

No idea how "rand" is supposed to work. Activating normal AES-ECB works, but CCM mode and transparent packet encryption/decryption haven't been working yet.

## RFEND

* +0x00
    * reset value 0
    * cannot write
* +0x04
	* reset 0x00010000
	* can set to 0x00011111
	* bit 0 = TX coarse tuning 1 freq
	* bit 4 = TX coarse tuning overall
	* bit 8 = TX fine tuning
	* bit 12 = RX filter tuning?
	* bit 16 = RX ADC tuning?
* +0x08
	* reset 0x000019f8
	* can set to 0x00731ff8
* +0x0c
	* reset 0x00001111
	* cannot set any more bits
	* can clear
* +0x10
	* reset 0x140108ff
	* can set 0x3f1fbfff
	* used for LL_SetTxPowerLevel
* +0x14
    * always zero?
* +0x18
    * always zero?
* +0x1c
    * always zero?
* +0x20
    * always zero?
* +0x24
    * always zero?
* +0x28
	* reset 0x0000047f
	* can set 0x00007fff
* +0x2c
	* reset 0x02222000
	* can set 0x077f301f
* +0x30
	* reset 0x43344407
	* can set 0x777777ff
* +0x34
	* reset 0x00000453
	* can set 0x00000777
* +0x38
	* reset 0x4000d320
	* [30:24]		nGA tuning (@ center 2.440 GHz?)
	* [23:19]		DACREF_TUNE
	* [16:8]		frequency for tuning operation?
	* [5:0]		    nCO tuning (@ center 2.440 GHz?)
	* can set 0xfff9ff3f
* +0x3c
	* reset 0x6b0062c0
	* can set 0xff01f3ff
* +0x40
    * always zero?
* +0x44
	* reset 0x0060c000
	* can set 0x01f3ffff
	* related to setting custom freq out of channels
* +0x48
	* reset 0x44010209
	* can set 0xf717f31f
* +0x4c
	* reset 0x02221700
	* can set 0x03331777
* +0x50
	* reset 0x0041a010
	* can set 0x1071f71f
	* related to RX filter
* +0x54
	* reset 0x0000113a
	* can set 0x000117ff
    * related to raw carrier test mode?
* +0x58
	* reset 0x0021030a
	* can set 0x003107ff
	* related to RX ADC
* +0x5c
	* reset 0x74400000
	* can set 0x77771f0f
* +0x60
    * always zero?
* +0x64
    * always zero?
* +0x68
    * always zero?
* +0x6c
    * always zero?
* +0x70
    * always zero?
* +0x74
    * always zero?
* +0x78
    * always zero?
* +0x7c
    * always zero?
* +0x80
    * always zero?
* +0x84
    * always zero?
* +0x88
    * always zero?
* +0x8c
	* reset 0x000000ff
	* R/O?
    * related to RX ADC tuning
* +0x90
	* reset 0x00400000
	* R/O?
	* tuning status and CO feedback
	* checked during normal operations too
* +0x94
	* reset 0x0001000a
	* R/O?
	* tuning GA feedback
* +0x98
    * always zero?
* +0x9c
	* reset 0x00000010
	* R/O?
	* related to RX filter tuning feedback
* +0xa0 .. 0xc8
    * nCO tuning
* +0xc8 .. 0xd0
    * nGA tuning
* +0xd4
    * always zero?
* +0xd8
    * always zero?
* +0xdc
    * always zero?
* +0xe0
    * always zero?
* +0xe4
    * always zero?
* +0xe8
    * always zero?
* +0xec
    * always zero?
* +0xf0
    * always zero?
* +0xf4
    * always zero?
* +0xf8
    * always zero?
* +0xfc
    * always zero?

"nCO" tuning -> carrier offset?

"nGA" tuning -> gain?

The most sus part of this is +0x90 being accessed during normal packet transmission.