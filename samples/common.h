/*
 *  _____  _   _  _____  _  _  _
 * |_   _|| | | |/  ___|| |(_)| |     Steam
 *   | |  | |_| |\ `--. | | _ | |__     In-Home
 *   | |  |  _  | `--. \| || || '_ \      Streaming
 *  _| |_ | | | |/\__/ /| || || |_) |       Library
 *  \___/ \_| |_/\____/ |_||_||_.__/
 *
 * Copyright (c) 2022 Ningyuan Li <https://github.com/mariotaku>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <stdint.h>
#include "ihslib.h"

static const uint64_t deviceId = 6002176711230249339;
static const uint64_t steamId = 76561198142668823;

static const uint8_t secretKey[32] = {
        0x2b, 0xf7, 0x9a, 0xf4, 0xc3, 0x67, 0x9b, 0x47,
        0x07, 0xf1, 0xf0, 0x24, 0x28, 0xb6, 0x31, 0x91,
        0x30, 0xdb, 0x2e, 0xc7, 0x30, 0x70, 0x21, 0x34,
        0xb9, 0xa4, 0x36, 0xed, 0xd6, 0x1a, 0x4a, 0x0f,
};

static const char deviceName[] = "Steam Link\0";


static const IHS_ClientConfig clientConfig = {deviceId, secretKey, deviceName};