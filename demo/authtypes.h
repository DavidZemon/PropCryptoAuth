/**
 * @file    authtypes.h
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

#include <cstdint>

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
#define SLOT_CONFIGURATION(writeConfig, writeKey, isSecret, encryptRead, limitedUse, noMac, readKey) ( \
    /* High order byte */ \
    (((writeConfig) & 0x0f) << 12) | \
    (((writeKey) & 0x0f) << 4) | \
    \
    /* Low order byte */ \
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

class SlotConfig {
    public:
        static const SlotConfig PUBLIC_KEY;

    public:
        SlotConfig (unsigned int readKeyId,
                    bool disableMacCommand,
                    bool limitedUse,
                    bool encryptedRead,
                    bool isSecret,
                    unsigned int writeKeyId,
                    unsigned int writeConfig)
                : readKeyId(readKeyId),
                  disableMacCommand(disableMacCommand),
                  limitedUse(limitedUse),
                  encryptedRead(encryptedRead),
                  isSecret(isSecret),
                  writeKeyId(writeKeyId),
                  writeConfig(writeConfig) {
        }

        uint16_t raw () const {
            return *(uint16_t *) this;
        }

        // @formatter:off
        unsigned int    readKeyId:          4;
        bool            disableMacCommand:  1;
        bool            limitedUse:         1;
        bool            encryptedRead:      1;
        bool            isSecret:           1;
        unsigned int    writeKeyId:         4;
        unsigned int    writeConfig:        4;
        // @formatter:on
};

const SlotConfig SlotConfig::PUBLIC_KEY(1, false, false, false, true, 0, 0b0010);
