#ifndef CLASS_HPW5500_REGISTERS_H
#define CLASS_HPW5500_REGISTERS_H

/**
 * Mode Register (MR)
 */

#define HPW5500_ADDR_MR                                                     0x0000
#define HPW5500_BLOCK_MR                                                    HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_MR                                                      HPW5500_ADDR_MR, HPW5500_BLOCK_MR
#define HPW5500_LENGTH_MR                                                   1
#define HPW5500_DEFAULT_MR                                                  0x00
#define HPW5500_USABLE_MR                                                   0xBA
#define HPW5500_READONLY_MR                                                 false

#define HPW5500_MASK_MR_RST                                                 0x80 // 1 bit
#define HPW5500_MASK_MR_WOL                                                 0x20 // 1 bit
#define HPW5500_MASK_MR_PB                                                  0x10 // 1 bit
#define HPW5500_MASK_MR_PPPOE                                               0x08 // 1 bit
#define HPW5500_MASK_MR_FARP                                                0x02 // 1 bit

#define HPW5500_MASK_MR_RESERVED_BIT0                                       0x01 // 1 bit
#define HPW5500_MASK_MR_RESERVED_BIT2                                       0x04 // 1 bit
#define HPW5500_MASK_MR_RESERVED_BIT6                                       0x40 // 1 bit

#define HPW5500_VAL_MR_INDUCE_RST                                           0b1

#define HPW5500_VAL_MR_WOL_DISABLED                                         0b0
#define HPW5500_VAL_MR_WOL_ENABLED                                          0b1

#define HPW5500_VAL_MR_PB_DISABLED                                          0b0
#define HPW5500_VAL_MR_PB_ENABLED                                           0b1

#define HPW5500_VAL_MR_PPPOE_DISABLED                                       0b0
#define HPW5500_VAL_MR_PPPOE_ENABLED                                        0b1

#define HPW5500_VAL_MR_FARP_DISABLED                                        0b0
#define HPW5500_VAL_MR_FARP_ENABLED                                         0b1

/**
 * Gateway IP Address Register (GAR)
 */

#define HPW5500_ADDR_GAR                                                    0x0001
#define HPW5500_BLOCK_GAR                                                   HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_GAR                                                     HPW5500_ADDR_GAR, HPW5500_BLOCK_GAR
#define HPW5500_LENGTH_GAR                                                  4
#define HPW5500_DEFAULT_GAR                                                 0x00000000
#define HPW5500_USABLE_GAR                                                  0xFFFFFFFF
#define HPW5500_READONLY_GAR                                                false

/**
 * Subnet Mask Register (SUBR)
 */

#define HPW5500_ADDR_SUBR                                                   0x0005
#define HPW5500_BLOCK_SUBR                                                  HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_SUBR                                                    HPW5500_ADDR_SUBR, HPW5500_BLOCK_SUBR
#define HPW5500_LENGTH_SUBR                                                 4
#define HPW5500_DEFAULT_SUBR                                                0x00000000
#define HPW5500_USABLE_SUBR                                                 0xFFFFFFFF
#define HPW5500_READONLY_SUBR                                               false

/**
 * Source Hardware Address Register (SHAR)
 */

#define HPW5500_ADDR_SHAR                                                   0x0009
#define HPW5500_BLOCK_SHAR                                                  HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_SHAR                                                    HPW5500_ADDR_SHAR, HPW5500_BLOCK_SHAR
#define HPW5500_LENGTH_SHAR                                                 6
#define HPW5500_DEFAULT_SHAR                                                0x000000000000
#define HPW5500_USABLE_SHAR                                                 0xFFFFFFFFFFFF
#define HPW5500_READONLY_SHAR                                               false

/**
 * Source IP Address Register (SIPR)
 */

#define HPW5500_ADDR_SIPR                                                   0x000F
#define HPW5500_BLOCK_SIPR                                                  HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_SIPR                                                    HPW5500_ADDR_SIPR, HPW5500_BLOCK_SIPR
#define HPW5500_LENGTH_SIPR                                                 4
#define HPW5500_DEFAULT_SIPR                                                0x00000000
#define HPW5500_USABLE_SIPR                                                 0xFFFFFFFF
#define HPW5500_READONLY_SIPR                                               false

/**
 * Interrupt Low Level Timer Register (INTLEVEL)
 */

#define HPW5500_ADDR_INTLEVEL                                               0x0013
#define HPW5500_BLOCK_INTLEVEL                                              HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_INTLEVEL                                                HPW5500_ADDR_INTLEVEL, HPW5500_BLOCK_INTLEVEL
#define HPW5500_LENGTH_INTLEVEL                                             2
#define HPW5500_DEFAULT_INTLEVEL                                            0x0000
#define HPW5500_USABLE_INTLEVEL                                             0xFFFF
#define HPW5500_READONLY_INTLEVEL                                           false

/**
 * Interrupt Register (IR)
 */

#define HPW5500_ADDR_IR                                                     0x0015
#define HPW5500_BLOCK_IR                                                    HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_IR                                                      HPW5500_ADDR_IR, HPW5500_BLOCK_IR
#define HPW5500_LENGTH_IR                                                   1
#define HPW5500_DEFAULT_IR                                                  0x00
#define HPW5500_USABLE_IR                                                   0xF0
#define HPW5500_READONLY_IR                                                 false

#define HPW5500_MASK_IR_CONFLICT                                            0x80 // 1 bit
#define HPW5500_MASK_IR_UNREACH                                             0x40 // 1 bit
#define HPW5500_MASK_IR_PPPOE                                               0x20 // 1 bit
#define HPW5500_MASK_IR_MP                                                  0x10 // 1 bit

#define HPW5500_VAL_IR_EVENT_IDLE                                           0b0 // default
#define HPW5500_VAL_IR_EVENT_OCCURED                                        0b1

#define HPW5500_VAL_IR_ALL_RESET                                            HPW5500_USABLE_IR

/**
 * Interrupt Mask Register (IMR)
 */

#define HPW5500_ADDR_IMR                                                    0x0016
#define HPW5500_BLOCK_IMR                                                   HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_IMR                                                     HPW5500_ADDR_IMR, HPW5500_BLOCK_IMR
#define HPW5500_LENGTH_IMR                                                  1
#define HPW5500_DEFAULT_IMR                                                 0x00
#define HPW5500_USABLE_IMR                                                  0xF0
#define HPW5500_READONLY_IMR                                                false

#define HPW5500_MASK_IMR_CONFLICT                                           0x80 // 1 bit
#define HPW5500_MASK_IMR_UNREACH                                            0x40 // 1 bit
#define HPW5500_MASK_IMR_PPPOE                                              0x20 // 1 bit
#define HPW5500_MASK_IMR_MP                                                 0x10 // 1 bit

#define HPW5500_VAL_IMR_EVENT_DISABLED                                      0b0 // default
#define HPW5500_VAL_IMR_EVENT_ENABLED                                       0b1

#define HPW5500_VAL_IMR_ALL_DISABLED                                        HPW5500_DEFAULT_IMR
#define HPW5500_VAL_IMR_ALL_ENABLED                                         HPW5500_USABLE_IMR

/**
 * Socket Interrupt Register (SIR)
 */

#define HPW5500_ADDR_SIR                                                    0x0017
#define HPW5500_BLOCK_SIR                                                   HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_SIR                                                     HPW5500_ADDR_SIR, HPW5500_BLOCK_SIR
#define HPW5500_LENGTH_SIR                                                  1
#define HPW5500_DEFAULT_SIR                                                 0x00
#define HPW5500_USABLE_SIR                                                  0xFF
#define HPW5500_READONLY_SIR                                                false

#define HPW5500_MASK_SIR_S7                                                 0x80 // 1 bit
#define HPW5500_MASK_SIR_S6                                                 0x40 // 1 bit
#define HPW5500_MASK_SIR_S5                                                 0x20 // 1 bit
#define HPW5500_MASK_SIR_S4                                                 0x10 // 1 bit
#define HPW5500_MASK_SIR_S3                                                 0x08 // 1 bit
#define HPW5500_MASK_SIR_S2                                                 0x04 // 1 bit
#define HPW5500_MASK_SIR_S1                                                 0x02 // 1 bit
#define HPW5500_MASK_SIR_S0                                                 0x01 // 1 bit

#define HPW5500_VAL_SIR_EVENT_IDLE                                          0b0 // default
#define HPW5500_VAL_SIR_EVENT_OCCURED                                       0b1

#define HPW5500_VAL_SIR_ALL_RESET                                           HPW5500_USABLE_SIR

/**
 * Socket Interrupt Mask Register (SIMR)
 */

#define HPW5500_ADDR_SIMR                                                   0x0018
#define HPW5500_BLOCK_SIMR                                                  HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_SIMR                                                    HPW5500_ADDR_SIMR, HPW5500_BLOCK_SIMR
#define HPW5500_LENGTH_SIMR                                                 1
#define HPW5500_DEFAULT_SIMR                                                0x00
#define HPW5500_USABLE_SIMR                                                 0xFF
#define HPW5500_READONLY_SIMR                                               false

#define HPW5500_MASK_SIMR_S7                                                0x80 // 1 bit
#define HPW5500_MASK_SIMR_S6                                                0x40 // 1 bit
#define HPW5500_MASK_SIMR_S5                                                0x20 // 1 bit
#define HPW5500_MASK_SIMR_S4                                                0x10 // 1 bit
#define HPW5500_MASK_SIMR_S3                                                0x08 // 1 bit
#define HPW5500_MASK_SIMR_S2                                                0x04 // 1 bit
#define HPW5500_MASK_SIMR_S1                                                0x02 // 1 bit
#define HPW5500_MASK_SIMR_S0                                                0x01 // 1 bit

#define HPW5500_VAL_SIMR_EVENT_DISABLED                                     0b0 // default
#define HPW5500_VAL_SIMR_EVENT_ENABLED                                      0b1

#define HPW5500_VAL_SIMR_ALL_DISABLED                                       HPW5500_DEFAULT_SIMR
#define HPW5500_VAL_SIMR_ALL_ENABLED                                        HPW5500_USABLE_SIMR

/**
 * Retry Time-value Register (RTR)
 */

#define HPW5500_ADDR_RTR                                                    0x0019
#define HPW5500_BLOCK_RTR                                                   HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_RTR                                                     HPW5500_ADDR_RTR, HPW5500_BLOCK_RTR
#define HPW5500_LENGTH_RTR                                                  2
#define HPW5500_DEFAULT_RTR                                                 0x07D0
#define HPW5500_USABLE_RTR                                                  0xFFFF
#define HPW5500_READONLY_RTR                                                false

/**
 * PPP Link Control Protocol Request Timer Register (PTIMER)
 */

#define HPW5500_ADDR_PTIMER                                                 0x001C
#define HPW5500_BLOCK_PTIMER                                                HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_PTIMER                                                  HPW5500_ADDR_PTIMER, HPW5500_BLOCK_PTIMER
#define HPW5500_LENGTH_PTIMER                                               1
#define HPW5500_DEFAULT_PTIMER                                              0x28
#define HPW5500_USABLE_PTIMER                                               0xFF
#define HPW5500_READONLY_PTIMER                                             false

/**
 * PPP Link Control Protocol Magic number Register (PMAGIC)
 */

#define HPW5500_ADDR_PMAGIC                                                 0x001D
#define HPW5500_BLOCK_PMAGIC                                                HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_PMAGIC                                                  HPW5500_ADDR_PMAGIC, HPW5500_BLOCK_PMAGIC
#define HPW5500_LENGTH_PMAGIC                                               1
#define HPW5500_DEFAULT_PMAGIC                                              0x00
#define HPW5500_USABLE_PMAGIC                                               0xFF
#define HPW5500_READONLY_PMAGIC                                             false

/**
 * Destination Hardware Address Register in PPPoE mode (PHAR)
 */

#define HPW5500_ADDR_PHAR                                                   0x001E
#define HPW5500_BLOCK_PHAR                                                  HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_PHAR                                                    HPW5500_ADDR_PHAR, HPW5500_BLOCK_PHAR
#define HPW5500_LENGTH_PHAR                                                 6
#define HPW5500_DEFAULT_PHAR                                                0x000000000000
#define HPW5500_USABLE_PHAR                                                 0xFFFFFFFFFFFF
#define HPW5500_READONLY_PHAR                                               false

/**
 * Session ID Register in PPPoE mode (PSID)
 */

#define HPW5500_ADDR_PSID                                                   0x0024
#define HPW5500_BLOCK_PSID                                                  HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_PSID                                                    HPW5500_ADDR_PSID, HPW5500_BLOCK_PSID
#define HPW5500_LENGTH_PSID                                                 2
#define HPW5500_DEFAULT_PSID                                                0x0000
#define HPW5500_USABLE_PSID                                                 0xFFFF
#define HPW5500_READONLY_PSID                                               false

/**
 * Maximum Receive Unit in PPPoE mode (PMRU)
 */

#define HPW5500_ADDR_PMRU                                                   0x0026
#define HPW5500_BLOCK_PMRU                                                  HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_PMRU                                                    HPW5500_ADDR_PMRU, HPW5500_BLOCK_PMRU
#define HPW5500_LENGTH_PMRU                                                 2
#define HPW5500_DEFAULT_PMRU                                                0xFFFF
#define HPW5500_USABLE_PMRU                                                 0xFFFF
#define HPW5500_READONLY_PMRU                                               false

/**
 * Unreachable IP Address Register (UIPR)
 */

#define HPW5500_ADDR_UIPR                                                   0x0028
#define HPW5500_BLOCK_UIPR                                                  HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_UIPR                                                    HPW5500_ADDR_UIPR, HPW5500_BLOCK_UIPR
#define HPW5500_LENGTH_UIPR                                                 4
#define HPW5500_DEFAULT_UIPR                                                0x00000000
#define HPW5500_USABLE_UIPR                                                 0xFFFFFFFF
#define HPW5500_READONLY_UIPR                                               true

/**
 * Unreachable Port Register (UPORTR)
 */

#define HPW5500_ADDR_UPORTR                                                 0x002C
#define HPW5500_BLOCK_UPORTR                                                HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_UPORTR                                                  HPW5500_ADDR_UPORTR, HPW5500_BLOCK_UPORTR
#define HPW5500_LENGTH_UPORTR                                               2
#define HPW5500_DEFAULT_UPORTR                                              0x0000
#define HPW5500_USABLE_UPORTR                                               0xFFFF
#define HPW5500_READONLY_UPORTR                                             true

/**
 * W5500 PHY Configuration Register (PHYCFGR)
 */

#define HPW5500_ADDR_PHYCFGR                                                0x002E
#define HPW5500_BLOCK_PHYCFGR                                               HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_PHYCFGR                                                 HPW5500_ADDR_PHYCFGR, HPW5500_BLOCK_PHYCFGR
#define HPW5500_LENGTH_PHYCFGR                                              1
#define HPW5500_DEFAULT_PHYCFGR                                             0xB8
#define HPW5500_USABLE_PHYCFGR                                              0xF8
#define HPW5500_READONLY_PHYCFGR                                            false

#define HPW5500_MASK_PHYCFGR_RST                                            0x80 // 1 bit
#define HPW5500_MASK_PHYCFGR_OPMD                                           0x40 // 1 bit
#define HPW5500_MASK_PHYCFGR_OPMDC                                          0x38 // 3 bits
#define HPW5500_MASK_PHYCFGR_DPX                                            0x04 // 1 bit (read-only)
#define HPW5500_MASK_PHYCFGR_SPD                                            0x02 // 1 bit (read-only)
#define HPW5500_MASK_PHYCFGR_LNK                                            0x01 // 1 bit (read-only)

#define HPW5500_VAL_PHYCFGR_INDUCE_RST                                      0b0

#define HPW5500_VAL_PHYCFGR_OPMD_VIA_OPMDC                                  0b1
#define HPW5500_VAL_PHYCFGR_OPMD_VIA_HW_PINS                                0b0 // default

#define HPW5500_VAL_PHYCFGR_OPMDC_10BT_HD_AND                               0b000 // 10BT Half-duplex, Auto-negotiation disabled
#define HPW5500_VAL_PHYCFGR_OPMDC_10BT_FD_AND                               0b001 // 10BT Full-duplex, Auto-negotiation disabled
#define HPW5500_VAL_PHYCFGR_OPMDC_100BT_HD_AND                              0b010 // 100BT Half-duplex, Auto-negotiation disabled
#define HPW5500_VAL_PHYCFGR_OPMDC_100BT_FD_AND                              0b011 // 100BT Full-duplex, Auto-negotiation disabled
#define HPW5500_VAL_PHYCFGR_OPMDC_100BT_HD_ANE                              0b100 // 100BT Half-duplex, Auto-negotiation enabled
#define HPW5500_VAL_PHYCFGR_OPMDC_NOT_USED                                  0b101 // Not used
#define HPW5500_VAL_PHYCFGR_OPMDC_POWER_DOWN_MODE                           0b110 // Power Down mode
#define HPW5500_VAL_PHYCFGR_OPMDC_ALL_CAPABLE_ANE                           0b111 // All capable, Auto-negotiation enabled (default)

#define HPW5500_VAL_PHYCFGR_DPX_FD                                          0b1 // Full duplex
#define HPW5500_VAL_PHYCFGR_DPX_HF                                          0b0 // Half duplex

#define HPW5500_VAL_PHYCFGR_SPD_100                                         0b1 // 100Mbps based
#define HPW5500_VAL_PHYCFGR_SPD_10                                          0b0 // 10Mbps based

#define HPW5500_VAL_PHYCFGR_LNK_UP                                          0b1 // Link up
#define HPW5500_VAL_PHYCFGR_LNK_DOWN                                        0b0 // Link down

/**
 * Version register (VERSIONR)
 */

#define HPW5500_ADDR_VERSIONR                                               0x0039
#define HPW5500_BLOCK_VERSIONR                                              HPW5500_REGISTER_BLOCK_COMMON
#define HPW5500_COM_VERSIONR                                                HPW5500_ADDR_VERSIONR, HPW5500_BLOCK_VERSIONR
#define HPW5500_LENGTH_VERSIONR                                             1
#define HPW5500_DEFAULT_VERSIONR                                            0x04
#define HPW5500_USABLE_VERSIONR                                             0xFF
#define HPW5500_READONLY_VERSIONR                                           true

#define HPW5500_MASK_VERSIONR_VERSION                                       0xFF // 8 bits

/**
 * Socket n Mode Register (Sn_MR)
 */

#define HPW5500_ADDR_SN_MR                                                  0x0000
#define HPW5500_BLOCK_SN_MR(s)                                              HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_MR(s)                                                HPW5500_ADDR_SN_MR, HPW5500_BLOCK_SN_MR(s)
#define HPW5500_LENGTH_SN_MR                                                1
#define HPW5500_DEFAULT_SN_MR                                               0x00
#define HPW5500_USABLE_SN_MR                                                0xFF
#define HPW5500_READONLY_SN_MR                                              false

#define HPW5500_MASK_SN_MR_MULTI_MFEN                                       0x80 // 1 bit
#define HPW5500_MASK_SN_MR_BCASTB                                           0x40 // 1 bit
#define HPW5500_MASK_SN_MR_ND_MC_MMB                                        0x20 // 1 bit
#define HPW5500_MASK_SN_MR_UCASTB_MIP6B                                     0x10 // 1 bit
#define HPW5500_MASK_SN_MR_PROTOCOL                                         0x0F // 4 bits

#define HPW5500_VAL_SN_MR_MULTI_MFEN_MULTI_DISABLED                         0b0 // default; applies in UDP mode
#define HPW5500_VAL_SN_MR_MULTI_MFEN_MULTI_ENABLED                          0b1 // applies in UDP mode
#define HPW5500_VAL_SN_MR_MULTI_MFEN_MFEN_DISABLED                          0b0 // default; applies in MACRAW mode
#define HPW5500_VAL_SN_MR_MULTI_MFEN_MFEN_ENABLED                           0b1 // applies in MACRAW mode

#define HPW5500_VAL_SN_MR_BCASTB_DISABLED                                   0b0 // default; applies in MACRAW and UDP modes
#define HPW5500_VAL_SN_MR_BCASTB_ENABLED                                    0b1 // applies in MACRAW and UDP modes

#define HPW5500_VAL_SN_MR_ND_MC_MMB_ND_DISABLED                             0b0 // default; applies in TCP mode
#define HPW5500_VAL_SN_MR_ND_MC_MMB_ND_ENABLED                              0b1 // applies in TCP mode
#define HPW5500_VAL_SN_MR_ND_MC_MMB_MC_DISABLED                             0b0 // default; applies in UDP mode
#define HPW5500_VAL_SN_MR_ND_MC_MMB_MC_ENABLED                              0b1 // applies in UDP mode
#define HPW5500_VAL_SN_MR_ND_MC_MMB_MMB_DISABLED                            0b0 // default; applies in MACRAW mode
#define HPW5500_VAL_SN_MR_ND_MC_MMB_MMB_ENABLED                             0b1 // applies in MACRAW mode

#define HPW5500_VAL_SN_MR_UCASTB_MIP6B_UCASTB_DISABLED                      0b0 // default; applies in UDP mode
#define HPW5500_VAL_SN_MR_UCASTB_MIP6B_UCASTB_ENABLED                       0b1 // applies in UDP mode
#define HPW5500_VAL_SN_MR_UCASTB_MIP6B_MIP6B_DISABLED                       0b0 // default; applies in MACRAW mode
#define HPW5500_VAL_SN_MR_UCASTB_MIP6B_MIP6B_ENABLED                        0b1 // applies in MACRAW mode

#define HPW5500_VAL_SN_MR_PROTOCOL_CLOSED                                   0b0000 // default
#define HPW5500_VAL_SN_MR_PROTOCOL_TCP                                      0b0001
#define HPW5500_VAL_SN_MR_PROTOCOL_UDP                                      0b0010
#define HPW5500_VAL_SN_MR_PROTOCOL_MACRAW                                   0b0100 // Note: MACRAW should be only use with Socket 0

/**
 * Socket n Command Register (Sn_CR)
 */

#define HPW5500_ADDR_SN_CR                                                  0x0001
#define HPW5500_BLOCK_SN_CR(s)                                              HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_CR(s)                                                HPW5500_ADDR_SN_CR, HPW5500_BLOCK_SN_CR(s)
#define HPW5500_LENGTH_SN_CR                                                1
#define HPW5500_DEFAULT_SN_CR                                               0x00
#define HPW5500_USABLE_SN_CR                                                0xFF
#define HPW5500_READONLY_SN_CR                                              false

#define HPW5500_MASK_SN_CR_COMMAND                                          0xFF // 8 bits

#define HPW5500_VAL_SN_CR_COMMAND_CLEARED                                   0x00 // default; after W5500 processes the command, the value in Sn_CR changes to 0x00
#define HPW5500_VAL_SN_CR_COMMAND_OPEN                                      0x01
#define HPW5500_VAL_SN_CR_COMMAND_LISTEN                                    0x02 // valid in TCP mode only
#define HPW5500_VAL_SN_CR_COMMAND_CONNECT                                   0x04 // valid in TCP mode only
#define HPW5500_VAL_SN_CR_COMMAND_DISCON                                    0x08 // valid in TCP mode only
#define HPW5500_VAL_SN_CR_COMMAND_CLOSE                                     0x10
#define HPW5500_VAL_SN_CR_COMMAND_SEND                                      0x20
#define HPW5500_VAL_SN_CR_COMMAND_SEND_MAC                                  0x21 // valid in UDP mode only
#define HPW5500_VAL_SN_CR_COMMAND_SEND_KEEP                                 0x22 // valid in TCP mode only
#define HPW5500_VAL_SN_CR_COMMAND_RECV                                      0x40

/**
 * Socket n Interrupt Register (Sn_IR)
 */

#define HPW5500_ADDR_SN_IR                                                  0x0002
#define HPW5500_BLOCK_SN_IR(s)                                              HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_IR(s)                                                HPW5500_ADDR_SN_IR, HPW5500_BLOCK_SN_IR(s)
#define HPW5500_LENGTH_SN_IR                                                1
#define HPW5500_DEFAULT_SN_IR                                               0x00
#define HPW5500_USABLE_SN_IR                                                0x1F
#define HPW5500_READONLY_SN_IR                                              false // datasheet states as read-only, but as stated in the same section that writing to it clears interrupts 🤷‍♂️

#define HPW5500_MASK_SN_IR_SEND_OK                                          0x10 // 1 bit
#define HPW5500_MASK_SN_IR_TIMEOUT                                          0x08 // 1 bit
#define HPW5500_MASK_SN_IR_RECV                                             0x04 // 1 bit
#define HPW5500_MASK_SN_IR_DISCON                                           0x02 // 1 bit
#define HPW5500_MASK_SN_IR_CON                                              0x01 // 1 bit

#define HPW5500_VAL_SN_IR_ALL_RESET                                         HPW5500_USABLE_SN_IR

/**
 * Socket n Status Register (Sn_SR)
 */

#define HPW5500_ADDR_SN_SR                                                  0x0003
#define HPW5500_BLOCK_SN_SR(s)                                              HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_SR(s)                                                HPW5500_ADDR_SN_SR, HPW5500_BLOCK_SN_SR(s)
#define HPW5500_LENGTH_SN_SR                                                1
#define HPW5500_DEFAULT_SN_SR                                               0x00
#define HPW5500_USABLE_SN_SR                                                0x1F
#define HPW5500_READONLY_SN_SR                                              false // datasheet states as read-only, but as stated in the same section that writing to it clears interrupts 🤷‍♂️

#define HPW5500_VAL_SN_SR_SOCK_CLOSED                                       0x00 // default
#define HPW5500_VAL_SN_SR_SOCK_INIT                                         0x13
#define HPW5500_VAL_SN_SR_SOCK_LISTEN                                       0x14
#define HPW5500_VAL_SN_SR_SOCK_ESTABLISHED                                  0x17
#define HPW5500_VAL_SN_SR_SOCK_CLOSE_WAIT                                   0x1C
#define HPW5500_VAL_SN_SR_SOCK_UDP                                          0x22
#define HPW5500_VAL_SN_SR_SOCK_MACRAW                                       0x42

#define HPW5500_VAL_SN_SR_SOCK_SYNSENT                                      0x15
#define HPW5500_VAL_SN_SR_SOCK_SYNRECV                                      0x16
#define HPW5500_VAL_SN_SR_SOCK_FIN_WAIT                                     0x18
#define HPW5500_VAL_SN_SR_SOCK_CLOSING                                      0x1A
#define HPW5500_VAL_SN_SR_SOCK_TIME_WAIT                                    0x1B
#define HPW5500_VAL_SN_SR_SOCK_LAST_ACK                                     0x1D

/**
 * Socket n Source Port Register (Sn_PORT)
 */

#define HPW5500_ADDR_SN_PORT                                                0x0004
#define HPW5500_BLOCK_SN_PORT(s)                                            HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_PORT(s)                                              HPW5500_ADDR_SN_PORT, HPW5500_BLOCK_SN_PORT(s)
#define HPW5500_LENGTH_SN_PORT                                              2
#define HPW5500_DEFAULT_SN_PORT                                             0x0000
#define HPW5500_USABLE_SN_PORT                                              0xFFFF
#define HPW5500_READONLY_SN_PORT                                            false

/**
 * Socket n Destination Hardware Address Register (Sn_DHAR)
 */

#define HPW5500_ADDR_SN_DHAR                                                0x0006
#define HPW5500_BLOCK_SN_DHAR(s)                                            HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_DHAR(s)                                              HPW5500_ADDR_SN_DHAR, HPW5500_BLOCK_SN_DHAR(s)
#define HPW5500_LENGTH_SN_DHAR                                              6
#define HPW5500_DEFAULT_SN_DHAR                                             0xFFFFFFFFFFFF
#define HPW5500_USABLE_SN_DHAR                                              0xFFFFFFFFFFFF
#define HPW5500_READONLY_SN_DHAR                                            false

/**
 * Socket n Destination IP Address Register (Sn_DIPR)
 */

#define HPW5500_ADDR_SN_DIPR                                                0x000C
#define HPW5500_BLOCK_SN_DIPR(s)                                            HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_DIPR(s)                                              HPW5500_ADDR_SN_DIPR, HPW5500_BLOCK_SN_DIPR(s)
#define HPW5500_LENGTH_SN_DIPR                                              4
#define HPW5500_DEFAULT_SN_DIPR                                             0x00000000
#define HPW5500_USABLE_SN_DIPR                                              0xFFFFFFFF
#define HPW5500_READONLY_SN_DIPR                                            false

/**
 * Socket n Destination Port Register (Sn_DPORT)
 */

#define HPW5500_ADDR_SN_DPORT                                               0x0010
#define HPW5500_BLOCK_SN_DPORT(s)                                           HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_DPORT(s)                                             HPW5500_ADDR_SN_DPORT, HPW5500_BLOCK_SN_DPORT(s)
#define HPW5500_LENGTH_SN_DPORT                                             2
#define HPW5500_DEFAULT_SN_DPORT                                            0x0000
#define HPW5500_USABLE_SN_DPORT                                             0xFFFF
#define HPW5500_READONLY_SN_DPORT                                           false

/**
 * Socket n Maximum Segment Size Register (Sn_MSSR)
 */

#define HPW5500_ADDR_SN_MSSR                                                0x0012
#define HPW5500_BLOCK_SN_MSSR(s)                                            HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_MSSR(s)                                              HPW5500_ADDR_SN_MSSR, HPW5500_BLOCK_SN_MSSR(s)
#define HPW5500_LENGTH_SN_MSSR                                              2
#define HPW5500_DEFAULT_SN_MSSR                                             0x0000
#define HPW5500_USABLE_SN_MSSR                                              0xFFFF
#define HPW5500_READONLY_SN_MSSR                                            false

/**
 * Socket n IP Type of Service Register (Sn_TOS)
 */

#define HPW5500_ADDR_SN_TOS                                                 0x0015
#define HPW5500_BLOCK_SN_TOS(s)                                             HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_TOS(s)                                               HPW5500_ADDR_SN_TOS, HPW5500_BLOCK_SN_TOS(s)
#define HPW5500_LENGTH_SN_TOS                                               1
#define HPW5500_DEFAULT_SN_MSSR                                             0x00
#define HPW5500_USABLE_SN_MSSR                                              0xFF
#define HPW5500_READONLY_SN_MSSR                                            false

#define HPW5500_MASK_SN_TOS                                                 0xFF // 8 bits

/**
 * Socket n TTL Register (Sn_TTL)
 */

#define HPW5500_ADDR_SN_TTL                                                 0x0016
#define HPW5500_BLOCK_SN_TTL(s)                                             HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_TTL(s)                                               HPW5500_ADDR_SN_TTL, HPW5500_BLOCK_SN_TTL(s)
#define HPW5500_LENGTH_SN_TTL                                               1
#define HPW5500_DEFAULT_SN_TTL                                              0x80
#define HPW5500_USABLE_SN_TTL                                               0xFF
#define HPW5500_READONLY_SN_TTL                                             false

#define HPW5500_MASK_SN_TTL                                                 0xFF // 8 bits

/**
 * Socket n RX Buffer Size Register (Sn_RXBUF_SIZE)
 */

#define HPW5500_ADDR_SN_RXBUF_SIZE                                          0x001E
#define HPW5500_BLOCK_SN_RXBUF_SIZE(s)                                      HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_RXBUF_SIZE(s)                                        HPW5500_ADDR_SN_RXBUF_SIZE, HPW5500_BLOCK_SN_RXBUF_SIZE(s)
#define HPW5500_LENGTH_SN_RXBUF_SIZE                                        1
#define HPW5500_DEFAULT_SN_RXBUF_SIZE                                       0x02
#define HPW5500_USABLE_SN_RXBUF_SIZE                                        0xFF
#define HPW5500_READONLY_SN_RXBUF_SIZE                                      false

#define HPW5500_MASK_SN_RXBUF_SIZE                                          0xFF // 8 bits

#define HPW5500_VAL_SN_RXBUF_SIZE_0KB                                       0x00
#define HPW5500_VAL_SN_RXBUF_SIZE_1KB                                       0x01
#define HPW5500_VAL_SN_RXBUF_SIZE_2KB                                       0x02 // default
#define HPW5500_VAL_SN_RXBUF_SIZE_4KB                                       0x04
#define HPW5500_VAL_SN_RXBUF_SIZE_8KB                                       0x08
#define HPW5500_VAL_SN_RXBUF_SIZE_16KB                                      0x10

/**
 * Socket n TX Buffer Size Register (Sn_TXBUF_SIZE)
 */

#define HPW5500_ADDR_SN_TXBUF_SIZE                                          0x001F
#define HPW5500_BLOCK_SN_TXBUF_SIZE(s)                                      HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_TXBUF_SIZE(s)                                        HPW5500_ADDR_SN_TXBUF_SIZE, HPW5500_BLOCK_SN_TXBUF_SIZE(s)
#define HPW5500_LENGTH_SN_TXBUF_SIZE                                        1
#define HPW5500_DEFAULT_SN_TXBUF_SIZE                                       0x02
#define HPW5500_USABLE_SN_TXBUF_SIZE                                        0xFF
#define HPW5500_READONLY_SN_TXBUF_SIZE                                      false

#define HPW5500_MASK_SN_TXBUF_SIZE                                          0xFF // 8 bits

#define HPW5500_VAL_SN_TXBUF_SIZE_0KB                                       0x00
#define HPW5500_VAL_SN_TXBUF_SIZE_1KB                                       0x01
#define HPW5500_VAL_SN_TXBUF_SIZE_2KB                                       0x02 // default
#define HPW5500_VAL_SN_TXBUF_SIZE_4KB                                       0x04
#define HPW5500_VAL_SN_TXBUF_SIZE_8KB                                       0x08
#define HPW5500_VAL_SN_TXBUF_SIZE_16KB                                      0x10

/**
 * Socket n TX Free Size Register (Sn_TX_FSR)
 */

#define HPW5500_ADDR_SN_TX_FSR                                              0x0020
#define HPW5500_BLOCK_SN_TX_FSR(s)                                          HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_TX_FSR(s)                                            HPW5500_ADDR_SN_TX_FSR, HPW5500_BLOCK_SN_TX_FSR(s)
#define HPW5500_LENGTH_SN_TX_FSR                                            2
#define HPW5500_DEFAULT_SN_TX_FSR                                           0x0800
#define HPW5500_USABLE_SN_TX_FSR                                            0xFFFF
#define HPW5500_READONLY_SN_TX_FSR                                          true

/**
 * Socket n TX Read Pointer Register (Sn_TX_RD)
 */

#define HPW5500_ADDR_SN_TX_RD                                               0x0022
#define HPW5500_BLOCK_SN_TX_RD(s)                                           HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_TX_RD(s)                                             HPW5500_ADDR_SN_TX_RD, HPW5500_BLOCK_SN_TX_RD(s)
#define HPW5500_LENGTH_SN_TX_RD                                             2
#define HPW5500_DEFAULT_SN_TX_RD                                            0x0000
#define HPW5500_USABLE_SN_TX_RD                                             0xFFFF
#define HPW5500_READONLY_SN_TX_RD                                           true

/**
 * Socket n TX Write Pointer Register (Sn_TX_WR)
 */

#define HPW5500_ADDR_SN_TX_WR                                               0x0024
#define HPW5500_BLOCK_SN_TX_WR(s)                                           HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_TX_WR(s)                                             HPW5500_ADDR_SN_TX_WR, HPW5500_BLOCK_SN_TX_WR(s)
#define HPW5500_LENGTH_SN_TX_WR                                             2
#define HPW5500_DEFAULT_SN_TX_WR                                            0x0000
#define HPW5500_USABLE_SN_TX_WR                                             0xFFFF
#define HPW5500_READONLY_SN_TX_WR                                           false

/**
 * Socket n Received Size Register (Sn_RX_RSR)
 */

#define HPW5500_ADDR_SN_RX_RSR                                              0x0026
#define HPW5500_BLOCK_SN_RX_RSR(s)                                          HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_RX_RSR(s)                                            HPW5500_ADDR_SN_RX_RSR, HPW5500_BLOCK_SN_RX_RSR(s)
#define HPW5500_LENGTH_SN_RX_RSR                                            2
#define HPW5500_DEFAULT_SN_RX_RSR                                           0x0000
#define HPW5500_USABLE_SN_RX_RSR                                            0xFFFF
#define HPW5500_READONLY_SN_RX_RSR                                          true

/**
 * Socket n RX Read Data Pointer Register (Sn_RX_RD)
 */

#define HPW5500_ADDR_SN_RX_RD                                               0x0028
#define HPW5500_BLOCK_SN_RX_RD(s)                                           HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_RX_RD(s)                                             HPW5500_ADDR_SN_RX_RD, HPW5500_BLOCK_SN_RX_RD(s)
#define HPW5500_LENGTH_SN_RX_RD                                             2
#define HPW5500_DEFAULT_SN_RX_RD                                            0x0000
#define HPW5500_USABLE_SN_RX_RD                                             0xFFFF
#define HPW5500_READONLY_SN_RX_RD                                           false

/**
 * Socket n RX Write Pointer Register (Sn_RX_WR)
 */

#define HPW5500_ADDR_SN_RX_WR                                               0x002A
#define HPW5500_BLOCK_SN_RX_WR(s)                                           HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_RX_WR(s)                                             HPW5500_ADDR_SN_RX_WR, HPW5500_BLOCK_SN_RX_WR(s)
#define HPW5500_LENGTH_SN_RX_WR                                             2
#define HPW5500_DEFAULT_SN_RX_WR                                            0x0000
#define HPW5500_USABLE_SN_RX_WR                                             0xFFFF
#define HPW5500_READONLY_SN_RX_WR                                           true

/**
 * Socket n Interrupt Mask Register (Sn_IMR)
 */

#define HPW5500_ADDR_SN_IMR                                                 0x002C
#define HPW5500_BLOCK_SN_IMR(s)                                             HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_IMR(s)                                               HPW5500_ADDR_SN_IMR, HPW5500_BLOCK_SN_IMR(s)
#define HPW5500_LENGTH_SN_IMR                                               1
#define HPW5500_DEFAULT_SN_IMR                                              0xFF
#define HPW5500_USABLE_SN_IMR                                               0x1F
#define HPW5500_READONLY_SN_IMR                                             false

#define HPW5500_MASK_SN_IMR_SEND_OK                                         0x10 // 1 bit
#define HPW5500_MASK_SN_IMR_TIMEOUT                                         0x08 // 1 bit
#define HPW5500_MASK_SN_IMR_RECV                                            0x04 // 1 bit
#define HPW5500_MASK_SN_IMR_DISCON                                          0x02 // 1 bit
#define HPW5500_MASK_SN_IMR_CON                                             0x01 // 1 bit

#define HPW5500_VAL_SN_IMR_ALL_DISABLED                                     0x00
#define HPW5500_VAL_SN_IMR_ALL_ENABLED                                      HPW5500_USABLE_SN_IMR

/**
 * Socket n Fragment Register (Sn_FRAG)
 */

#define HPW5500_ADDR_SN_FRAG                                                0x002D
#define HPW5500_BLOCK_SN_FRAG(s)                                            HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_FRAG(s)                                              HPW5500_ADDR_SN_FRAG, HPW5500_BLOCK_SN_FRAG(s)
#define HPW5500_LENGTH_SN_FRAG                                              2
#define HPW5500_DEFAULT_SN_FRAG                                             0x4000
#define HPW5500_USABLE_SN_FRAG                                              0xFFFF
#define HPW5500_READONLY_SN_FRAG                                            false

#define HPW5500_VAL_SN_FRAG_DO_NOT_FRAGMENT                                 0x0000

/**
 * Socket n Keep Alive Time Register (Sn_KPALVTR)
 */

#define HPW5500_ADDR_SN_KPALVTR                                             0x002F
#define HPW5500_BLOCK_SN_KPALVTR(s)                                         HPW5500_REGISTER_BLOCK_SOCK(s)
#define HPW5500_COM_SN_KPALVTR(s)                                           HPW5500_ADDR_SN_KPALVTR, HPW5500_BLOCK_SN_KPALVTR(s)
#define HPW5500_LENGTH_SN_KPALVTR                                           1
#define HPW5500_DEFAULT_SN_KPALVTR                                          0x00
#define HPW5500_USABLE_SN_KPALVTR                                           0xFF
#define HPW5500_READONLY_SN_KPALVTR                                         false

#endif // CLASS_HPW5500_REGISTERS_H
