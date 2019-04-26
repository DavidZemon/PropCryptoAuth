/**
 * @file    common.cpp
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

#include "common.h"

#include <PropWare/utility/utility.h>
#include <PropWare/hmi/output/printer.h>
#include <atca_basic.h>

using PropWare::Utility;

const Printer::Format HEX_FMT(2, '0', 16);

void print_block (const Printer &printer, const uint8_t *const data, const size_t length) {
    char buffer[256];
    auto bufferSize = Utility::size_of_array(buffer);
    atcab_bin2hex(data, length, buffer, &bufferSize);
    printer << buffer;
}

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
