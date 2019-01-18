//
// Created by david on 1/17/19.
//

#include <atca_hal.h>
#include <atca_device.h>
#include <atca_execution.h>
#include <string.h>

ATCA_STATUS hal_i2c_discover_devices (int bus_num, ATCAIfaceCfg cfg[], int *found) {
    ATCAIfaceCfg *head        = cfg;
    uint8_t      slaveAddress = 0x01;
    ATCADevice   device;

#ifdef ATCA_NO_HEAP
    struct atca_device disc_device;
    struct atca_command disc_command;
    struct atca_iface disc_iface;
#endif
    ATCAPacket  packet;
    ATCA_STATUS status;
    uint8_t     revs608[][4]  = {{0x00, 0x00, 0x60, 0x01}, {0x00, 0x00, 0x60, 0x02}};
    uint8_t     revs508[][4]  = {{0x00, 0x00, 0x50, 0x00}};
    uint8_t     revs108[][4]  = {{0x80, 0x00, 0x10, 0x01}};
    uint8_t     revs204[][4]  = {{0x00, 0x02, 0x00, 0x08}, {0x00, 0x02, 0x00, 0x09}, {0x00, 0x04, 0x05, 0x00}};
    int         i;

    // default configuration, to be reused during discovery process
    ATCAIfaceCfg discoverCfg = {
            .iface_type             = ATCA_I2C_IFACE,
            .devtype                = ATECC508A,
            .atcai2c.slave_address  = 0x07,
            .atcai2c.bus            = bus_num,
            .atcai2c.baud           = 400000,
            //.atcai2c.baud = 100000,
            .wake_delay             = 800,
            .rx_retries             = 3
    };

    if (bus_num < 0) {
        puts("bus_num = bad value = ");
        putDec(bus_num);
        putchar('\n');
        return ATCA_COMM_FAIL;
    }

#ifdef ATCA_NO_HEAP
    disc_device.mCommands = &disc_command;
    disc_device.mIface    = &disc_iface;
    status = initATCADevice(&discoverCfg, &disc_device);
    if (status != ATCA_SUCCESS)
    {
        return status;
    }
    device = &disc_device;
#else
    device = newATCADevice(&discoverCfg);
    if (device == NULL) {
        puts("Device is NULL!!!\n");
        return ATCA_COMM_FAIL;
    }
#endif

    // iterate through all addresses on given i2c bus
    // all valid 7-bit addresses go from 0x07 to 0x78
    for (slaveAddress = 0x07; slaveAddress <= 0x78; slaveAddress++) {
        // turn it into an 8-bit address which is what the rest of the i2c HAL is expecting when a packet is sent
        discoverCfg.atcai2c.slave_address = slaveAddress << 1;
        if (discoverCfg.atcai2c.slave_address != 0xC8)
            continue;

        memset(packet.data, 0x00, sizeof(packet.data));
        // build an info command
        packet.param1 = INFO_MODE_REVISION;
        packet.param2 = 0;
        // get devrev info and set device type accordingly
        atInfo(device->mCommands, &packet);
        if ((status = atca_execute_command(&packet, device)) != ATCA_SUCCESS) {
            puts("No response 0x");
            puthex((int) slaveAddress);
            putchar('\n');
            continue;
        } else {
            puts("Found 0x");
            puthex((int) slaveAddress);
            putchar('\n');
        }

        // determine device type from common info and dev rev response byte strings... start with unknown
        discoverCfg.devtype = ATCA_DEV_UNKNOWN;
        for (i = 0; i < (int) sizeof(revs608) / 4; i++) {
            if (memcmp(&packet.data[1], &revs608[i], 4) == 0) {
                discoverCfg.devtype = ATECC608A;
                break;
            }
        }

        for (i = 0; i < (int) sizeof(revs508) / 4; i++) {
            if (memcmp(&packet.data[1], &revs508[i], 4) == 0) {
                discoverCfg.devtype = ATECC508A;
                break;
            }
        }

        for (i = 0; i < (int) sizeof(revs204) / 4; i++) {
            if (memcmp(&packet.data[1], &revs204[i], 4) == 0) {
                discoverCfg.devtype = ATSHA204A;
                break;
            }
        }

        for (i = 0; i < (int) sizeof(revs108) / 4; i++) {
            if (memcmp(&packet.data[1], &revs108[i], 4) == 0) {
                discoverCfg.devtype = ATECC108A;
                break;
            }
        }

        if (discoverCfg.devtype != ATCA_DEV_UNKNOWN) {
            // now the device type is known, so update the caller's cfg array element with it
            (*found)++;
            memcpy((uint8_t *) head, (uint8_t *) &discoverCfg, sizeof(ATCAIfaceCfg));
            head->devtype = discoverCfg.devtype;
            head++;
        }

        atca_delay_ms(15);
    }

#ifdef ATCA_NO_HEAP
    releaseATCADevice(device);
#else
    deleteATCADevice(&device);

#endif

    return ATCA_SUCCESS;
}
