#include <driver/i2c.h>
#include <esp_log.h>
#include <libesp.h>
#include <libi2c.h>

#include "esp-as7341.h"

static const char* TAG = "as7341";

esp_err_t as7341_init(i2c_port_t port, uint8_t addr, as7341_handle_t* out_dev) {
    as7341_handle_t dev;
    i2c_7bit_init(port, addr, &dev);

    // As per spec, there is a power-on reset (POR) which occurs on startup,
    // typically 200us. During this time the device will NAK us, so we cannot
    // distinguish from an unconntected/improperly connected device.
    util_wait_micros(500);

    uint8_t reg_id;
    if (i2c_7bit_reg_read(dev, AS7341_REGB_HI_ID, 1, &reg_id) != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed, are I2C pin numbers/address correct?");
        goto as7341_init_fail;
    }

    uint8_t reg_auxid = as7341_regb_hibank_read(dev, AS7341_REGB_HI_AUXID);
    uint8_t reg_revid = as7341_regb_hibank_read(dev, AS7341_REGB_HI_REVID);

    if ((reg_id & MASK_AS7341_ID_DRIVER_SUPPORTED) != AS7341_ID_DRIVER_SUPPORTED || (reg_auxid & MASK_AS7341_AUXID_DRIVER_SUPPORTED) != AS7341_AUXID_DRIVER_SUPPORTED || (reg_revid & MASK_AS7341_REVID_DRIVER_SUPPORTED) != AS7341_REVID_DRIVER_SUPPORTED) {
        ESP_LOGE(TAG, "unsupported device IDs (0x%02X, 0x%02X, 0x%02X), have you specified the address of another device?", reg_id, reg_auxid, reg_revid);
        goto as7341_init_fail;
    }

    as7341_regb_hibank_write(dev, AS7341_REGB_HI_ENABLE, AS7341_HI_ENABLE_PON);

    *out_dev = dev;
    return ESP_OK;

as7341_init_fail:
    i2c_7bit_destroy(dev);
    return ESP_FAIL;
}

void as7341_destroy(as7341_handle_t dev) {
    i2c_7bit_destroy(dev);
}

static uint8_t regb_read(as7341_handle_t dev, uint8_t reg) {
    uint8_t val;
    ESP_ERROR_CHECK(i2c_7bit_reg_read(dev, reg, 1, &val));

    ESP_LOGD(TAG, "reg_readb(0x%02X)=0x%02X", reg, val);
    return val;
}

static uint16_t regw_read(as7341_handle_t dev, uint8_t reg) {
    // As per the spec the low byte must be read before the high byte
    // (which happens naturally due to autoincrementing together with
    // order of the assigned register numbers), else there will be
    // errors.

    // An internal latch ensures that both bytes are always
    // synchronized (i.e. there is no race between the read of the
    // second byte and a potential simultaneous update.)

    uint8_t data[2];
    ESP_ERROR_CHECK(i2c_7bit_reg_read(dev, reg, 2, data));

    uint16_t val = (((uint16_t) data[0]) << 0) | (((uint16_t) data[1]) << 8);

    ESP_LOGD(TAG, "reg_readw(0x%02X)=0x%04X", reg, val);
    return val;
}

static void regb_write(as7341_handle_t dev, uint8_t reg, uint8_t val) {
    ESP_ERROR_CHECK(i2c_7bit_reg_write(dev, reg, 1, &val));

    ESP_LOGD(TAG, "reg_writeb(0x%02X)=0x%02X", reg, val);
}

static void regw_write(as7341_handle_t dev, uint8_t reg, uint16_t val) {
    // As per the spec the low byte must be write before the high byte
    // (which happens naturally due to autoincrementing together with
    // order of the assigned register numbers), else there will be
    // errors.

    uint8_t data[2];
    data[0] = (val & 0x00FF) >> 0;
    data[1] = (val & 0xFF00) >> 8;
    ESP_ERROR_CHECK(i2c_7bit_reg_write(dev, reg, 2, data));

    ESP_LOGD(TAG, "reg_writew(0x%02X)=0x%04X", reg, val);
}

static void set_lobank_enabled(as7341_handle_t dev, bool en) {
    uint8_t reg_cfg0 = as7341_regb_hibank_read(dev, AS7341_REGB_HI_CFG_0);
    as7341_regb_hibank_write(dev, AS7341_REGB_HI_CFG_0, (reg_cfg0 & ~AS7341_HI_CFG_0_REG_BANK) | (en ? AS7341_HI_CFG_0_REG_BANK : 0));
}

uint8_t as7341_regb_lobank_read(as7341_handle_t dev, as7341_regb_lobank_t reg) {
    set_lobank_enabled(dev, true);
    uint8_t ret = regb_read(dev, reg);
    set_lobank_enabled(dev, false);
    return ret;
}

uint16_t as7341_regw_lobank_read(as7341_handle_t dev, as7341_regw_lobank_t reg) {
    set_lobank_enabled(dev, true);
    uint16_t ret = regw_read(dev, reg);
    set_lobank_enabled(dev, false);
    return ret;
}

uint8_t as7341_regb_hibank_read(as7341_handle_t dev, as7341_regb_hibank_t reg) {
    return regb_read(dev, reg);
}

uint16_t as7341_regw_hibank_read(as7341_handle_t dev, as7341_regw_hibank_t reg) {
    return regw_read(dev, reg);
}

void as7341_regb_lobank_write(as7341_handle_t dev, as7341_regb_lobank_t reg, uint8_t val) {
    set_lobank_enabled(dev, true);
    regb_write(dev, reg, val);
    set_lobank_enabled(dev, false);
}

void as7341_regw_lobank_write(as7341_handle_t dev, as7341_regw_lobank_t reg, uint16_t val) {
    set_lobank_enabled(dev, true);
    regw_write(dev, reg, val);
    set_lobank_enabled(dev, false);
}

void as7341_regb_hibank_write(as7341_handle_t dev, as7341_regb_hibank_t reg, uint8_t val) {
    regb_write(dev, reg, val);
}

void as7341_regw_hibank_write(as7341_handle_t dev, as7341_regw_hibank_t reg, uint16_t val) {
    regw_write(dev, reg, val);
}

static void smux_enable_and_wait_for_completion(as7341_handle_t dev) {
    as7341_regb_hibank_write(dev, AS7341_REGB_HI_ENABLE, AS7341_HI_ENABLE_PON | AS7341_HI_ENABLE_SMUXEN);

    // Wait until command execution completes, detected by the clearing of SMUXEN.
    while (as7341_regb_hibank_read(dev, AS7341_REGB_HI_ENABLE) & AS7341_HI_ENABLE_SMUXEN) {
        vTaskDelay(1);
    }
}

void as7341_smux_setup_lo_channels(as7341_handle_t dev) {
    as7341_regb_hibank_write(dev, AS7341_REGB_HI_CFG_6, AS7341_HI_CFG_6_SMUX_CMD_WRITE);

    // Magic incantation to configure the SMUX for:
    // * CH0 = F1
    // * CH1 = F2
    // * CH2 = F3
    // * CH3 = F4
    // * CH4 = CLEAR
    // * CH5 = NIR
    regb_write(dev, 0x00, 0x30);
    regb_write(dev, 0x01, 0x01);
    regb_write(dev, 0x02, 0x00);
    regb_write(dev, 0x03, 0x00);
    regb_write(dev, 0x04, 0x00);
    regb_write(dev, 0x05, 0x42);
    regb_write(dev, 0x06, 0x00);
    regb_write(dev, 0x07, 0x00);
    regb_write(dev, 0x08, 0x50);
    regb_write(dev, 0x09, 0x00);
    regb_write(dev, 0x0A, 0x00);
    regb_write(dev, 0x0B, 0x00);
    regb_write(dev, 0x0C, 0x20);
    regb_write(dev, 0x0D, 0x04);
    regb_write(dev, 0x0E, 0x00);
    regb_write(dev, 0x0F, 0x30);
    regb_write(dev, 0x10, 0x01);
    regb_write(dev, 0x11, 0x50);
    regb_write(dev, 0x12, 0x00);
    regb_write(dev, 0x13, 0x06);

    smux_enable_and_wait_for_completion(dev);
}

void as7341_smux_setup_hi_channels(as7341_handle_t dev) {
    as7341_regb_hibank_write(dev, AS7341_REGB_HI_CFG_6, AS7341_HI_CFG_6_SMUX_CMD_WRITE);

    // Magic incantation to configure the SMUX for:
    // * CH0 = F5
    // * CH1 = F6
    // * CH2 = F7
    // * CH3 = F8
    // * CH4 = CLEAR
    // * CH5 = NIR
    regb_write(dev, 0x00, 0x00);
    regb_write(dev, 0x01, 0x00);
    regb_write(dev, 0x02, 0x00);
    regb_write(dev, 0x03, 0x40);
    regb_write(dev, 0x04, 0x02);
    regb_write(dev, 0x05, 0x00);
    regb_write(dev, 0x06, 0x10);
    regb_write(dev, 0x07, 0x03);
    regb_write(dev, 0x08, 0x50);
    regb_write(dev, 0x09, 0x10);
    regb_write(dev, 0x0A, 0x03);
    regb_write(dev, 0x0B, 0x00);
    regb_write(dev, 0x0C, 0x00);
    regb_write(dev, 0x0D, 0x00);
    regb_write(dev, 0x0E, 0x24);
    regb_write(dev, 0x0F, 0x00);
    regb_write(dev, 0x10, 0x00);
    regb_write(dev, 0x11, 0x50);
    regb_write(dev, 0x12, 0x00);
    regb_write(dev, 0x13, 0x06);

    smux_enable_and_wait_for_completion(dev);
}
