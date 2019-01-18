//
// Created by david on 1/16/19.
//

#include <cryptoauthlib.h>
#include <atca_hal.h>
#include <PropWare/serial/i2c/i2cmaster.h>
#include <PropWare/hmi/output/printer.h>

using PropWare::I2CMaster;
using PropWare::Printer;

extern "C" {

ATCA_STATUS hal_i2c_init (void *hal, ATCAIfaceCfg *cfg) {
    pwOut << "Made it to the HAL init!\n";
    if (cfg->atcai2c.bus > 1) {
        // No support for multiple I2C buses yet... just use the same one as the EEPROM because it's easy
        pwOut << "bus is bad number " << (int) cfg->atcai2c.bus << '\n';
        return ATCA_COMM_FAIL;
    }
    pwI2c.set_frequency(cfg->atcai2c.baud);
    ((ATCAHAL_t *) hal)->hal_data = &pwI2c;
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_post_init (ATCAIface iface) {
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_send (ATCAIface iface, uint8_t *txdata, int txlength) {
    const auto cfg = atgetifacecfg(iface);
    const auto i2c = (I2CMaster *) atgetifacehaldat(iface);

    pwOut << "Attempting to send the following string of " << txlength "bytes:\n" << Printer::Format(2, '0', 16);
    pwOut << "\t0 = 0x03\n";
    for (unsigned int i = 1; i < txlength; ++i) {
        pwOut << "\t" << i << " = 0x" << (unsigned int) txdata[i] << '\n';
    }
    pwOut << Printer::DEFAULT_FORMAT;

    // The Microchip library inserts an extra (blank) byte into the txdata array for us to insert the 0x03. The
    // PropWare library does not expect that at all and therefore we send txdata starting with txdata[1]
    if (i2c->put(cfg->atcai2c.slave_address, static_cast<uint8_t>(0x03), &txdata[1], static_cast<size_t>(txlength - 1)))
        return ATCA_SUCCESS;
    else
        return ATCA_TX_TIMEOUT;
}

ATCA_STATUS hal_i2c_receive (ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength) {
    ATCA_STATUS status;
    const auto  cfg           = atgetifacecfg(iface);
    const auto  i2c           = (I2CMaster *) atgetifacehaldat(iface);
    const auto  rxDataMaxSize = *rxlength;
    auto        retries       = cfg->rx_retries;

    *rxlength = 0; // Set the actual length now so that any errors report accurate return values
    if (!rxDataMaxSize) {
        pwOut << "WTF\n";
        return ATCA_SMALL_BUFFER;
    }

    do {
        i2c->start();
        const auto success = i2c->send_byte(cfg->atcai2c.slave_address | 0x01);
        if (success) {
            const auto expectedDataLength = rxdata[0] = i2c->read_byte(true);
            if (expectedDataLength < ATCA_RSP_SIZE_MIN) {
                i2c->stop();
                status = ATCA_INVALID_SIZE;
                break;
            }
            if (expectedDataLength > rxDataMaxSize) {
                i2c->stop();
                pwOut << "Can not receive " << expectedDataLength << " bytes into " << rxDataMaxSize << " byte array\n";
                status = ATCA_SMALL_BUFFER;
                break;
            }

            size_t i;
            for (i = 1; i < (expectedDataLength - 1); ++i)
                rxdata[i] = i2c->read_byte(true);
            rxdata[i]     = i2c->read_byte(false);
            i2c->stop();
            *rxlength = expectedDataLength;
            status = ATCA_SUCCESS;
        } else {
            status = ATCA_COMM_FAIL;
        }
    } while (--retries > 0 && status != ATCA_SUCCESS);

    return status;
}

ATCA_STATUS hal_i2c_wake (ATCAIface iface) {
    const auto cfg = atgetifacecfg(iface);
    const auto i2c = (I2CMaster *) atgetifacehaldat(iface);

    uint8_t  data[]   = {0x00, 0x00, 0x00, 0x00};
    uint16_t dataSize = sizeof(data);

    // Waking the device requires holding SDA low for a period of time. Easiest way to do this is to set the
    // frequency to 100kHz and then ping address 0. We're not looking for a response - just want to hold SDA low for
    // a little while.
    i2c->set_frequency(100000);
    i2c->ping(0); // Wake the device
    i2c->set_frequency(cfg->atcai2c.baud);

    waitcnt(CNT + MICROSECOND * cfg->wake_delay);
    if (!i2c->ping(cfg->atcai2c.slave_address)) {
        return ATCA_TIMEOUT;
    } else {
        const auto status = hal_i2c_receive(iface, data, &dataSize);
        if (status != ATCA_SUCCESS)
            return status;
        else
            return hal_check_wake(data, dataSize);
    }
}

ATCA_STATUS hal_i2c_idle (ATCAIface iface) {
    const auto cfg = atgetifacecfg(iface);
    const auto i2c = (I2CMaster *) atgetifacehaldat(iface);
    if (i2c->put(cfg->atcai2c.slave_address, 0x02))
        return ATCA_SUCCESS;
    else
        return ATCA_TX_TIMEOUT;
}

ATCA_STATUS hal_i2c_sleep (ATCAIface iface) {
    const auto cfg = atgetifacecfg(iface);
    const auto i2c = (I2CMaster *) atgetifacehaldat(iface);
    if (i2c->put(cfg->atcai2c.slave_address, 0x01))
        return ATCA_SUCCESS;
    else
        return ATCA_TX_TIMEOUT;
}

ATCA_STATUS hal_i2c_release (void *hal_data) {
    return ATCA_SUCCESS;
}

void atca_delay_us (uint32_t delay) {
    waitcnt(CNT + MICROSECOND * delay);
}

void atca_delay_10us (uint32_t delay) {
    waitcnt(CNT + MICROSECOND * 10 * delay);
}

void atca_delay_ms (uint32_t delay) {
    waitcnt(CNT + MILLISECOND * delay);
}

ATCA_STATUS hal_i2c_discover_buses (int i2c_buses[], int max_buses) {
    i2c_buses[0] = 1;
    return ATCA_SUCCESS;
}

}
