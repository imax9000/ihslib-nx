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

#include "channel.h"
#include "protobuf/remoteplay.pb-c.h"
#include "session/frame.h"

typedef struct IHS_SessionChannelData {
    IHS_SessionChannel base;
    IHS_SessionPacketsWindow *window;
    uv_thread_t workerThread;
    bool threadInterrupted;
    uv_mutex_t mutex;
    uv_cond_t cond;
} IHS_SessionChannelData;

typedef struct IHS_SessionChannelDataClass {
    IHS_SessionChannelClass base;

    void (*start)(struct IHS_SessionChannel *channel);

    void (*received)(struct IHS_SessionChannel *channel, const IHS_SessionFrame *frame);

    void (*stop)(struct IHS_SessionChannel *channel);
} IHS_SessionChannelDataClass;

IHS_SessionChannel *IHS_SessionChannelDataCreate(const IHS_SessionChannelDataClass *cls, IHS_Session *session,
                                                 IHS_SessionChannelType type, IHS_SessionChannelId id,
                                                 const void *config);


void IHS_SessionChannelDataInit(IHS_SessionChannel *channel);

void IHS_SessionChannelDataDeinit(IHS_SessionChannel *channel);

void IHS_SessionChannelDataReceived(struct IHS_SessionChannel *channel, const IHS_SessionPacket *packet);

void IHS_SessionChannelControlOnDataControl(IHS_SessionChannel *channel, EStreamControlMessage type,
                                            const uint8_t *payload, size_t payloadLen,
                                            const IHS_SessionPacketHeader *header);