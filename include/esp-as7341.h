#ifndef __LIB_AS7341_H
#define __LIB_AS7341_H

#include <driver/i2c.h>
#include <libi2c.h>

// The device ID/AUXID/REVID of the AS7341 supported by this driver.
#define AS7341_ID_DRIVER_SUPPORTED 0b00100100
#define MASK_AS7341_ID_DRIVER_SUPPORTED 0b11111100

#define AS7341_AUXID_DRIVER_SUPPORTED 0b00000000
#define MASK_AS7341_AUXID_DRIVER_SUPPORTED 0b00001111

#define AS7341_REVID_DRIVER_SUPPORTED 0b00000000
#define MASK_AS7341_REVID_DRIVER_SUPPORTED 0b00000111

typedef enum as7341_regb_lobank {
    AS7341_REGB_LO_ASTATUS = 0x60,

    // DANGER: as per spec, must read these three in this order---
    // as with all of the 2-byte (word) size registers, but we handle
    // the 2-byte case inside the driver.
    AS7341_REGB_LO_ITIME_L = 0x63,
    AS7341_REGB_LO_ITIME_M = 0x64,
    AS7341_REGB_LO_ITIME_H = 0x65,

    AS7341_REGB_LO_CONFIG = 0x70,
    AS7341_REGB_LO_STAT = 0x71,
    AS7341_REGB_LO_EDGE = 0x72,
    AS7341_REGB_LO_GPIO = 0x73,
    AS7341_REGB_LO_LED = 0x74,
} as7341_regb_lobank_t;

typedef enum as7341_regb_hibank {
    AS7341_REGB_HI_ENABLE = 0x80,
    AS7341_REGB_HI_ATIME = 0x81,
    AS7341_REGB_HI_WTIME = 0x83,

    AS7341_REGB_HI_AUXID = 0x90,
    AS7341_REGB_HI_REVID = 0x91,
    AS7341_REGB_HI_ID = 0x92,

    AS7341_REGB_HI_STATUS = 0x93,
    AS7341_REGB_HI_ASTATUS = 0x94,

    AS7341_REGB_HI_STATUS_2 = 0xA3,
    AS7341_REGB_HI_STATUS_3 = 0xA4,
    AS7341_REGB_HI_STATUS_5 = 0xA6,
    AS7341_REGB_HI_STATUS_6 = 0xA7,

    AS7341_REGB_HI_CFG_0 = 0xA9,
    AS7341_REGB_HI_CFG_1 = 0xAA,
    AS7341_REGB_HI_CFG_3 = 0xAC,
    AS7341_REGB_HI_CFG_6 = 0xAF,
    AS7341_REGB_HI_CFG_8 = 0xB1,
    AS7341_REGB_HI_CFG_9 = 0xB2,
    AS7341_REGB_HI_CFG_10 = 0xB3,
    AS7341_REGB_HI_CFG_12 = 0xB5,

    AS7341_REGB_HI_PERS = 0xBD,
    AS7341_REGB_HI_GPIO_2 = 0xBE,

    AS7341_REGB_HI_AGC_GAIN_MAX = 0xCF,

    AS7341_REGB_HI_AZ_CONFIG = 0xD6,
    AS7341_REGB_HI_FD_TIME_1 = 0xD8,
    AS7341_REGB_HI_FD_TIME_2 = 0xDA,
    AS7341_REGB_HI_FD_CFG0 = 0xD7,
    AS7341_REGB_HI_FD_STATUS = 0xDB,

    AS7341_REGB_HI_FD_INTENAB = 0xF9,
    AS7341_REGB_HI_FD_CONTROL = 0xFA,
    AS7341_REGB_HI_FIFO_MAP = 0xFC,
    AS7341_REGB_HI_FIFO_LVL = 0xFD,
} as7341_regb_hibank_t;

typedef enum as7341_regw_lobank {
    AS7341_REGW_LO_CH0_DATA = 0x61,
    AS7341_REGW_LO_CH1_DATA = 0x66,
    AS7341_REGW_LO_CH2_DATA = 0x68,
    AS7341_REGW_LO_CH3_DATA = 0x6A,
    AS7341_REGW_LO_CH4_DATA = 0x6C,
    AS7341_REGW_LO_CH5_DATA = 0x6E,
} as7341_regw_lobank_t;

typedef enum as7341_regw_hibank {
    AS7341_REGW_HI_SP_TH_L = 0x84,
    AS7341_REGW_HI_SP_TH_H = 0x86,

    AS7341_REGW_HI_CH0_DATA = 0x95,
    AS7341_REGW_HI_CH1_DATA = 0x97,
    AS7341_REGW_HI_CH2_DATA = 0x99,
    AS7341_REGW_HI_CH3_DATA = 0x9B,
    AS7341_REGW_HI_CH4_DATA = 0x9D,
    AS7341_REGW_HI_CH5_DATA = 0x9F,

    AS7341_REGW_HI_ASTEP = 0xCA,

    AS7341_REGW_HI_FDATA = 0xFE,
} as7341_regw_hibank_t;

#define AS7341_LO_CONFIG_LED_SEL (1ULL << 3)

#define AS7341_LO_LED_LED_ACT (1ULL << 7)

// `led_drive` sets the LED drive current by the formula
//      `(led_drive + 2) * 2 mA`
static __always_inline uint8_t MK_AS7341_LO_LED(bool act, uint8_t led_drive) {
    assert(!(led_drive & 0x80));
    return (act ? AS7341_LO_LED_LED_ACT : 0) | led_drive;
}

// DANGER: The datasheet specifies this as bit 4 on the register
// page, but bit 3 on the summary page!
#define AS7341_HI_CFG_0_REG_BANK (1ULL << 4)

#define AS7341_HI_ENABLE_PON (1ULL << 0)
#define AS7341_HI_ENABLE_SP_EN (1ULL << 1)
#define AS7341_HI_ENABLE_SMUXEN (1ULL << 4)

#define AS7341_HI_CFG_6_SMUX_CMD_ROM (0b00ULL << 3)
#define AS7341_HI_CFG_6_SMUX_CMD_READ (0b01ULL << 3)
#define AS7341_HI_CFG_6_SMUX_CMD_WRITE (0b10ULL << 3)

#define AS7341_HI_STATUS_2_AVALID (1ULL << 6)

#define AS7341_HI_CFG_1_AGAIN_x0_5 0
#define AS7341_HI_CFG_1_AGAIN_x1 1
#define AS7341_HI_CFG_1_AGAIN_x2 2
#define AS7341_HI_CFG_1_AGAIN_x4 3
#define AS7341_HI_CFG_1_AGAIN_x8 4
#define AS7341_HI_CFG_1_AGAIN_x16 5
#define AS7341_HI_CFG_1_AGAIN_x32 6
#define AS7341_HI_CFG_1_AGAIN_x64 7
#define AS7341_HI_CFG_1_AGAIN_x128 8
#define AS7341_HI_CFG_1_AGAIN_x256 9
#define AS7341_HI_CFG_1_AGAIN_x512 10

typedef i2c_7bit_handle_t as7341_handle_t;

// Register the AS7341 on the given I2C bus.
esp_err_t as7341_init(i2c_port_t port, uint8_t addr, as7341_handle_t* out_dev);

// Release the given handle.
void as7341_destroy(as7341_handle_t dev);

// Read a register (byte/word) over I2C.
uint8_t as7341_regb_lobank_read(as7341_handle_t dev, as7341_regb_lobank_t reg);
uint8_t as7341_regb_hibank_read(as7341_handle_t dev, as7341_regb_hibank_t reg);
uint16_t as7341_regw_lobank_read(as7341_handle_t dev, as7341_regw_lobank_t reg);
uint16_t as7341_regw_hibank_read(as7341_handle_t dev, as7341_regw_hibank_t reg);

// Write a register (byte/word) over I2C.
void as7341_regb_lobank_write(as7341_handle_t dev, as7341_regb_lobank_t reg, uint8_t val);
void as7341_regb_hibank_write(as7341_handle_t dev, as7341_regb_hibank_t reg, uint8_t val);
void as7341_regw_lobank_write(as7341_handle_t dev, as7341_regw_lobank_t reg, uint16_t val);
void as7341_regw_hibank_write(as7341_handle_t dev, as7341_regw_hibank_t reg, uint16_t val);

// Configure the SMUX for:
// * CH0 = F1
// * CH1 = F2
// * CH2 = F3
// * CH3 = F4
// * CH4 = CLEAR
// * CH5 = NIR
void as7341_smux_setup_lo_channels(as7341_handle_t dev);

// Configure the SMUX for:
// * CH0 = F5
// * CH1 = F6
// * CH2 = F7
// * CH3 = F8
// * CH4 = CLEAR
// * CH5 = NIR
void as7341_smux_setup_hi_channels(as7341_handle_t dev);

#endif
