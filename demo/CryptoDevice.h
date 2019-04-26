/**
 * @file    CryptoDevice.h
 *
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2019 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included in all copies or substantial portions
 * of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include "common.h"

#include <atca_basic.h>
#include <PropWare/hmi/output/printer.h>

using PropWare::Printer;

class CryptoDevice {
    public:
        static const uint8_t        SHA256_DEFAULT_ADDRESS = 0xC0;
        static const ATCADeviceType DEFAULT_DEVICE_TYPE    = ATECC508A;
        static const uint32_t       DEFAULT_BAUD           = 1000000;
        static const uint8_t        DEFAULT_BUS            = 0;
        static const uint16_t       DEFAULT_WAKE_DELAY     = 800;
        static const uint8_t        DEFAULT_RX_RETRIES     = 3;

    public:
        CryptoDevice (const uint8_t slaveAddress = SHA256_DEFAULT_ADDRESS,
                      const ATCADeviceType deviceType = DEFAULT_DEVICE_TYPE,
                      const uint32_t baud = DEFAULT_BAUD,
                      const uint8_t bus = DEFAULT_BUS,
                      const uint16_t wakeDelay = DEFAULT_WAKE_DELAY,
                      const uint8_t rxRetries = DEFAULT_RX_RETRIES) {
            this->m_configuration.iface_type            = ATCA_I2C_IFACE;
            this->m_configuration.devtype               = deviceType;
            this->m_configuration.atcai2c.slave_address = slaveAddress;
            this->m_configuration.atcai2c.baud          = baud;
            this->m_configuration.atcai2c.bus           = bus;
            this->m_configuration.wake_delay            = wakeDelay;
            this->m_configuration.rx_retries            = rxRetries;
        }

        /**
         * @brief Prepare the library's state variables
         *
         * No wake is necessary - the library will issue a wake before each command is sent
         *
         * @return 0 upon success, error code otherwise
         */
        PropWare::ErrorCode initialize () {
            return atcab_init(&this->m_configuration);
        }

        PropWare::ErrorCode sleep () {
            PropWare::ErrorCode err;
            check_errors(atcab_sleep());
            check_errors(atcab_release());
            return 0;
        }

        PropWare::ErrorCode print_serial (const Printer &printer) const {
            PropWare::ErrorCode err;
            uint8_t             serialNumber[ATCA_SERIAL_NUM_SIZE];
            check_errors(atcab_read_serial_number(serialNumber));
            printer << "Serial number: ";
            print_block(printer, serialNumber, ATCA_SERIAL_NUM_SIZE);
            return 0;
        }

        PropWare::ErrorCode generate_key (const uint16_t keyId, const Printer *const printer = NULL) {
            const auto err = atcab_genkey(keyId, this->m_publicKey);
            if (printer) {
                if (err) {
                    printer->println("KEY GEN ERROR");
                } else {
                    print_block(*printer, this->m_publicKey, ATCA_PUB_KEY_SIZE);
                }
            }
            return err;
        }

    protected:
        ATCAIfaceCfg m_configuration;
        uint8_t      m_publicKey[ATCA_PUB_KEY_SIZE];
};
