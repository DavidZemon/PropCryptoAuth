/**
 * @file    common.h
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

#include <PropWare/hmi/output/printer.h>

#include <atca_basic.h>

using PropWare::Printer;

#define SPLIT(x) static_cast<uint8_t>(x), static_cast<uint8_t>(x >> 8)

#define SWAP(x) static_cast<uint16_t>((x << 8) | (x & 0xff))

extern const Printer::Format HEX_FMT;

void print_block (const Printer &printer, const uint8_t *const data, const size_t length);

Printer &operator<< (Printer &printer, const ATCADeviceType deviceType);

Printer &operator<< (Printer &printer, const ATCAIfaceCfg &cfg);
