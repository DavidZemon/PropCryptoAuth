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

#include <PropWare/utility/utility.h>
#include <PropWare/hmi/output/printer.h>
#include <atca_basic.h>

using PropWare::Utility;
using PropWare::Printer;

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

PropWare::ErrorCode initialize () {
    PropWare::ErrorCode err;

    ATCAIfaceCfg sha256;
    sha256.iface_type            = ATCA_I2C_IFACE;
    sha256.devtype               = ATSHA204A;
    sha256.atcai2c.slave_address = 0xC8;
    sha256.atcai2c.baud          = 400000;
    sha256.atcai2c.bus           = 0;
    sha256.wake_delay            = 800;
    sha256.rx_retries            = 3;

    err = atcab_init(&sha256);
    if (err) {
        pwOut << "Unable to initialize device:\n" << sha256;
        return err;
    }

    // Print serial number
    uint8_t serialNumber[ATCA_SERIAL_NUM_SIZE];
    check_errors(atcab_read_serial_number(serialNumber));
    pwOut << "Serial number: 0x";
    for (int i = ATCA_SERIAL_NUM_SIZE; i; --i)
        pwOut.put_int(serialNumber[i], 16, 2, '0');
    pwOut << '\n';

    return 0;
}

int main () {
    const auto err = initialize();

    pwOut << "COMPLETE! Status code = " << err << '\n';

    return 0;
}
