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

using PropWare::Utility;
using PropWare::Printer;
using PropWare::SimplePort;
using PropWare::Pin;
using PropWare::I2CMaster;

static const Printer::Format HEX_FMT(2, '0', 16);

extern "C" {
void putchar (char c) {
    pwOut << c;
}
void putChar (char c) {
    pwOut << c;
}
int  putStr(const char* str) {
    pwOut << str;
    return 0;
}
int  puts(const char* str) {
    pwOut << str;
    return 0;
}
void puthex(int x) {
    pwOut << HEX_FMT << x << Printer::DEFAULT_FORMAT;
}
}

Printer &operator<< (Printer &printer, const ATCAIfaceCfg &cfg) {
    printer << "ATCAIfaceCfg\n"
            << "  iface_type=" << cfg.iface_type << '\n'
            << "  devtype=" << cfg.devtype << '\n'
            << "  atcai2c\n"
            << "    slave_address=" << cfg.atcai2c.slave_address << '\n'
            << "    bus=" << cfg.atcai2c.bus << '\n'
            << "    baud=" << cfg.atcai2c.baud << '\n'
            << "  wake_delay=" << cfg.wake_delay << '\n'
            << "  rx_retries=" << cfg.rx_retries << '\n'
            << "  cfg_data=0x" << HEX_FMT << cfg.cfg_data << Printer::DEFAULT_FORMAT << '\n';

    return printer;
}

static int discover (void) {
    PropWare::ErrorCode err;
    ATCAIfaceCfg        ifaceCfgs[10];
    int                 i;
    const char          *devname[] = {"ATSHA204A", "ATAES132A", "ATECC508A", "ATECC608A"};  // indexed by ATCADeviceType

    for (i = 0; i < (int) (Utility::size_of_array(ifaceCfgs)); i++) {
        ifaceCfgs[i].devtype    = ATCA_DEV_UNKNOWN;
        ifaceCfgs[i].iface_type = ATCA_UNKNOWN_IFACE;
    }

    pwOut.printf("Searching...\n");
    check_errors(atcab_cfg_discover(ifaceCfgs, Utility::size_of_array(ifaceCfgs)));
    pwOut.printf("Searching for %d configurations\n", Utility::size_of_array(ifaceCfgs));
    for (i = 0; i < (int) (Utility::size_of_array(ifaceCfgs)); i++) {
        if (ifaceCfgs[i].devtype != ATCA_DEV_UNKNOWN) {
            pwOut.printf("Found %s ", devname[ifaceCfgs[i].devtype]);
            if (ifaceCfgs[i].iface_type == ATCA_I2C_IFACE) {
                pwOut.printf("@ bus %d addr %02x", ifaceCfgs[i].atcai2c.bus, ifaceCfgs[i].atcai2c.slave_address);
            }
            if (ifaceCfgs[i].iface_type == ATCA_SWI_IFACE) {
                pwOut.printf("@ bus %d", ifaceCfgs[i].atcaswi.bus);
            }
            pwOut.printf("\n");
        } else {
            pwOut << "Device " << i << " not found\n";
        }
    }

    return 0;
}

//static ATCA_STATUS get_info (uint8_t *revision) {
//    ATCA_STATUS status;
//
//    status = atcab_init(gCfg);
//    if (status != ATCA_SUCCESS) {
//        printf("atcab_init() failed with ret=0x%08X\r\n", status);
//        return status;
//    }
//
//    status = atcab_info(revision);
//    atcab_release();
//    if (status != ATCA_SUCCESS) {
//        printf("atcab_info() failed with ret=0x%08X\r\n", status);
//    }
//
//    return status;
//}
//
//static void info (void) {
//    ATCA_STATUS status;
//    uint8_t     revision[4];
//    char        displaystr[15];
//    size_t      displaylen = sizeof(displaystr);
//
//    status = get_info(revision);
//    if (status == ATCA_SUCCESS) {
//        // dump revision
//        atcab_bin2hex(revision, 4, displaystr, &displaylen);
//        pwOut.printf("revision:\r\n%s\r\n", displaystr);
//    }
//}

int do_stuff () {
    PropWare::ErrorCode err;


    const SimplePort leds (SimplePort::Mask::P16,
    8, SimplePort::Dir::OUT);
    leds.clear();

    unsigned int step = 0;
    leds.write(++step);

    char version[20];
    check_errors(atcab_version(version));
    if (20 <= strlen(version)) {
        for (size_t i = 0; i < Utility::size_of_array(version); ++i) {
            pwOut << "version[" << i << "] = " << version[i] << '\n';
        }
    } else {
        pwOut << "Version = " << version << '\n';
    }

    leds.write(++step);

    ATCAIfaceCfg cfg_array[4];
    check_errors(atcab_cfg_discover(cfg_array, Utility::size_of_array(cfg_array)));

    leds.write(++step);

    uint8_t deviceRevision[4];
    check_errors(atcab_info(deviceRevision));

    leds.write(++step);

    pwOut << "Device revision: 0x";
    for (size_t i = 4; i; --i) {
        pwOut << HEX_FMT << deviceRevision[i];
    }
    pwOut << Printer::DEFAULT_FORMAT << '\n';

    return 0;
}

int main () {
    //const auto err = do_stuff();

    const auto err = discover();
    pwOut << "COMPLETE. Status = 0x" << HEX_FMT << err << '\n';

    while (1);

    return 0;
}
