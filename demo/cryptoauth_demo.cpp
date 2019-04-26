/**
 * @file    cryptoauth_demo.cpp
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

#include "CryptoDevice.h"
#include "common.h"
#include "authtypes.h"

#include <PropWare/hmi/output/printer.h>
#include <PropWare/memory/blockstorage.h>

#include <atca_basic.h>

using PropWare::Printer;
using PropWare::Utility;
using PropWare::BlockStorage;

#define PUBLIC_KEY_CONFIG SLOT_CONFIGURATION(0b0010, 0, SLOT_IS_SECRET, SLOT_IS_PLAIN_TEXT, SLOT_USE_UNLIMITED, \
                                      NO_USAGE_RESTRICTIONS, 1)

/**
 * @brief Swap the bytes and split them into two distinct parameters that can be added as literals in a byte-array
 */
#define _PUB_KEY SPLIT(SWAP(SlotConfig::PUBLIC_KEY.raw()))

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
 *
 * Before messing with your device, do a read of your configuration zone data and save it off. This is the results of my
 * read, prior to any writes:
 *
 *          0  1  2  3  4  5  6  7    8  9  A  B  C  D  E  F
 * 0x0000: 01 23 62 53 00 00 50 00 - 55 DA F7 A0 EE C0 49 00 .#bS..P.U.....I.
 * 0x0010: C0 00 55 00 83 20 87 20 - 8F 20 C4 8F 8F 8F 8F 8F ..U.. . . ......
 * 0x0020: 9F 8F AF 8F 00 00 00 00 - 00 00 00 00 00 00 00 00 ................
 * 0x0030: 00 00 AF 8F FF FF FF FF - 00 00 00 00 FF FF FF FF ................
 * 0x0040: 00 00 00 00 FF FF FF FF - FF FF FF FF FF FF FF FF ................
 * 0x0050: FF FF FF FF 00 00 55 55 - FF FF 00 00 00 00 00 00 ......UU........
 * 0x0060: 33 00 33 00 33 00 1C 00 - 1C 00 1C 00 1C 00 1C 00 3.3.3...........
 * 0x0070: 3C 00 3C 00 3C 00 3C 00 - 3C 00 3C 00 3C 00 1C 00 <.<.<.<.<.<.<...
 */
// @formatter:off
static const uint8_t CONFIG_ZONE_DATA_ATECC508A[] = {
//            0     1     2     3     4     5     6     7       8     9     A     B     C     D     E     F
/* 0x00 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x10 */ 0xC0, 0x00, 0x55, 0x00,

// Special slot configuration (bytes 20-51)
/* 0x10 */                         _PUB_KEY  , 0x87, 0x20,   0x8F, 0x20, 0xC4, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F,
/* 0x20 */ 0x9F, 0x8F, 0xAF, 0x8F, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 0x30 */ 0x00, 0x00, 0xAF, 0x8F,

// Normal configuration again
/* 0x30 */                         0xFF, 0xFF, 0xFF, 0xFF,   0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x40 */ 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 0x50 */ 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x55, 0x55,   0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

// Special key configuration (bytes 96-127)
/* 0x60 */ 0x33, 0x00, 0x33, 0x00, 0x33, 0x00, 0x1C, 0x00,   0x1C, 0x00, 0x1C, 0x00, 0x1C, 0x00, 0x1C, 0x00,
/* 0x70 */ 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00,   0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x1C, 0x00
};
// @formatter:on

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

    pwOut << "Slot config: 0x";
    pwOut.put_int(PUBLIC_KEY_CONFIG, 16, 4, '0');
    pwOut << '\n';

    pwOut << "Slot config: 0x";
    pwOut.put_int(SlotConfig::PUBLIC_KEY.raw(), 16, 4, '0');
    pwOut << '\n';

    //pwOut << "Writing modified configuration zone data\n";
    //check_errors(atcab_write_config_zone(CONFIG_ZONE_DATA_ATECC508A));
    //pwOut << "Final configuration zone:\n";
    //check_errors(atcab_read_config_zone(configData));
    //BlockStorage::print_block(pwOut, configData, configDataSize);

    //pwOut << "Generating new public/private key: ";
    //check_errors(cryptoDevice.generate_key(1, &pwOut));
    //pwOut << '\n';

    return 0;
}

int main () {
    CryptoDevice cryptoDevice;
    const auto   err = run(cryptoDevice);
    pwOut << "COMPLETE! Status code = 0x" << HEX_FMT << err << '\n';
    cryptoDevice.sleep();
    return 0;
}
