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
#include <PropWare/gpio/pin.h>
#include <PropWare/gpio/simpleport.h>
#include <PropWare/memory/eeprom.h>

#include <atca_iface.h>
#include <atca_basic.h>
#include <atca_hal.h>
#include <atca_status.h>
#include "../cryptoauthlib/lib/atca_iface.h"
#include "../cryptoauthlib/lib/basic/atca_basic.h"

using PropWare::Utility;
using PropWare::Printer;
using PropWare::SimplePort;
using PropWare::Pin;
using PropWare::I2CMaster;

extern I2CMaster g_i2c;

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

PropWare::ErrorCode discover () {
    PropWare::ErrorCode err;

    ATCAIfaceCfg ifaceCfgs[4];
    check_errors(atcab_cfg_discover(ifaceCfgs, Utility::size_of_array(ifaceCfgs)));
    for (size_t i = 0; i < Utility::size_of_array(ifaceCfgs); ++i)
        if (ifaceCfgs[i].devtype != ATCA_DEV_UNKNOWN)
            pwOut << "Found one!\n" << ifaceCfgs[i] << '\n';

    err = atcab_release();
    if (err) {
        pwOut << "Failed to release after discovery!!! Error code = " << err << '\n';
    }
    return err;
}

int do_stuff () {
    PropWare::ErrorCode err;

    check_errors(discover());

    return 0;
}

int main () {
    const auto err = do_stuff();

    pwOut << "COMPLETE! Status code = " << err << '\n';

    return 0;
}
