/**
 * @file    cryptoauth_demo.cpp
 *
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
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

#include <PropWare/hmi/output/printer.h>
#include <PropWare/memory/blockstorage.h>
#include <atca_basic.h>

using PropWare::Printer;
using PropWare::Utility;
using PropWare::BlockStorage;

#define BYTE_SWAP(x) (((x & 0xff00) >> 8) | (x & 0xff))

/**
 * @brief Combine slot configuration values into a two-byte slot configuration
 *
 * @param[in] writeConfig
 * @param[in] writeKey
 * @param[in] isSecret          Instance of IsSecret
 * @param[in] encryptRead       Instance of IsEncrypted
 * @param[in] limitedUse        Instance of SlotMultiUseLimitation
 * @param[in] noMac             Instance of SlotMacAllowed
 * @param[in] readKey           Either a slot ID for the key to be used for encrypting reads of this slot or
 *                              combination of `(EcdhOutputTarget | EcdhPermission | MessageSignatureEnable |
 *                              ExternalSignatureEnable)`
 */
#define SLOT_CONFIGURATION(writeConfig, writeKey, isSecret, encryptRead, limitedUse, noMac, readKey) BYTE_SWAP( \
    ((writeConfig & 0x0f) << 12) | \
    ((writeKey & 0x0f) << 8) | \
    isSecret | \
    encryptRead | \
    limitedUse | \
    noMac | \
    (readKey & 0x0f) \
)

typedef enum {
    SLOT_IS_PUBLIC,
    SLOT_IS_SECRET = 0x80
} IsSecret;

typedef enum {
    SLOT_IS_PLAIN_TEXT,
    SLOT_IS_ENCRYPTED = 0x40
} IsEncrypted;

typedef enum {
    SLOT_USE_UNLIMITED,
    SLOT_USE_LIMITED = 0x20
} SlotMultiUseLimitation;

typedef enum {
    NO_USAGE_RESTRICTIONS,
    VERFICATION_ONLY = 0x10
} SlotMacAllowed;

typedef enum {
    ECDH_WRITE_NORMAL,
    ECDH_WRITE_TO_SLOT = 0x08
} EcdhOutputTarget;

typedef enum {
    ECDH_OP_FORBIDDEN,
    ECDH_OP_PERMITTED = 0x04
} EcdhPermission;

typedef enum {
    DISABLE_MSG_SIGNATURES,
    ENABLE_MSG_SIGNATURES = 0x02
} MessageSignatureEnable;

typedef enum {
    DISABLE_EXT_SIGNATURES,
    ENABLE_EXT_SIGNATURES = 0x01
} ExternalSignatureEnable;

/**
 * @brief Complete configuration data for the ATECC508A
 *
 * Data can only be written to devices in 4- or 32-byte chunks, so it's not worth trying to write the configuration
 * in a byte-by-byte manner. For this reason, we'll just configure the entire configuration zone at once and hardcode
 * a big block.
 *
 * 0xFF will be used for the read-only bytes serial number and revision information
 *
 * @note This method is NOT ideal because it prevents the program from computing a valid CRC for the configuration
 *       data block. It would be better to read the configuration zone at runtime, then set only the bits/bytes that
 *       matter for the application, calculate a CRC, and then write the data back with the checksum included.
 */
// @formatter:off
static const uint8_t CONFIG_ZONE_DATA_ATECC508A[] = {
//            0     1     2     3     4     5     6     7       8     9     A     B     C     D     E     F
/* 0x00 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x10 */ 0xC0, 0x00, 0x55, 0x00,

// Special slot configuration (bytes 20-51)
/* 0x10 */                         DEFLT_SLOT, 0x87, 0x20,   0x8F, 0x20, 0xC4, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F,
/* 0x20 */ 0x9F, 0x8F, 0xAF, 0x8F, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x30 */ 0x00, 0x00, 0xAF, 0x8F,

// Normal configuration again
/* 0x30 */                         0xFF, 0xFF, 0xFF, 0xFF,   0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x40 */ 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x50 */ 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x55, 0x55,   0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x60 */ 0x33, 0x00, 0x33, 0x00, 0x33, 0x00, 0x1C, 0x00,   0x1C, 0x00, 0x1C, 0x00, 0x1C, 0x00, 0x1C, 0x00,
/* 0x70 */ 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00,   0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x1C, 0x00
};
// @formatter:on

static const Printer::Format HEX_FMT(2, '0', 16);

Printer &operator<< (Printer &printer, const ATCADeviceType deviceType) {
    switch (deviceType) {
        case ATSHA204A:
            pwOut << "ATSHA204A";
            break;
        case ATECC108A:
            pwOut << "ATECC108A";
            break;
        case ATECC508A:
            pwOut << "ATECC508A";
            break;
        case ATECC608A:
            pwOut << "ATECC608A";
            break;
        default:
            pwOut << "UNKNOWN";
    }
    return printer;
}

Printer &operator<< (Printer &printer, const ATCAIfaceCfg &cfg) {
    printer << "ATCAIfaceCfg\n"
            << "  iface_type=" << cfg.iface_type << '\n'
            << "  devtype=" << cfg.devtype << '\n'
            << "  atcai2c\n"
            << "    slave_address=0x" << HEX_FMT << cfg.atcai2c.slave_address << Printer::DEFAULT_FORMAT << '\n'
            << "    bus=" << cfg.atcai2c.bus << '\n'
            << "    baud=" << cfg.atcai2c.baud << '\n'
            << "  wake_delay=" << cfg.wake_delay << '\n'
            << "  rx_retries=" << cfg.rx_retries << '\n'
            << "  cfg_data=0x" << Printer::Format(8, '0', 16) << (uint32_t) cfg.cfg_data << Printer::DEFAULT_FORMAT;

    return printer;
}

void print_block (const Printer &printer, const uint8_t *const data, const size_t length) {
    char buffer[256];
    auto bufferSize = Utility::size_of_array(buffer);
    atcab_bin2hex(data, length, buffer, &bufferSize);
    printer << buffer;
}

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

PropWare::ErrorCode run (CryptoDevice &cryptoDevice) {
    PropWare::ErrorCode err;

    check_errors(cryptoDevice.initialize());
    pwOut << "initialized\n";
    check_errors(cryptoDevice.print_serial(pwOut));
    pwOut << '\n';

    pwOut << "Initial configuration zone:\n";
    uint8_t    configData[128];
    const auto configDataSize = Utility::size_of_array(configData);
    check_errors(atcab_read_config_zone(configData));
    BlockStorage::print_block(pwOut, configData, configDataSize);

    check_errors(atcab_write_config_zone(CONFIG_ZONE_DATA_ATECC508A));

    pwOut << "Final configuration zone:\n";
    uint8_t    configData[128];
    const auto configDataSize = Utility::size_of_array(configData);
    check_errors(atcab_read_config_zone(configData));
    BlockStorage::print_block(pwOut, configData, configDataSize);

    pwOut << "Generating new public/private key: ";
    check_errors(cryptoDevice.generate_key(1, &pwOut));
    pwOut << '\n';

    return 0;
}

int main () {
    CryptoDevice cryptoDevice;
    const auto   err = run(cryptoDevice);
    pwOut << "COMPLETE! Status code = 0x" << HEX_FMT << err << '\n';
    cryptoDevice.sleep();
    return 0;
}
