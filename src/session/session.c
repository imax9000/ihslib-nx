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

#include <stdlib.h>
#include <memory.h>

#include "ihslib/session.h"
#include "ihslib/common.h"
#include "base.h"
#include "packet.h"
#include "session_pri.h"
#include "session/channels/channel.h"
#include "session/channels/ch_discovery.h"
#include "session/channels/ch_control.h"
#include "session/channels/ch_stats.h"

static void SessionRecvCallback(uv_udp_t *handle, ssize_t nread, uv_buf_t buf, struct sockaddr *addr, unsigned flags);

IHS_Session *IHS_SessionCreate(const IHS_ClientConfig *clientConfig, const IHS_SessionConfig *sessionConfig) {
    IHS_Session *session = malloc(sizeof(IHS_Session));
    memset(session, 0, sizeof(IHS_Session));
    IHS_BaseInit(&session->base, clientConfig, SessionRecvCallback, 0);
    session->config = *sessionConfig;
    session->numChannels = 3;
    session->channels[IHS_SessionChannelIdDiscovery] = IHS_SessionChannelDiscoveryCreate(session);
    session->channels[IHS_SessionChannelIdControl] = IHS_SessionChannelControlCreate(session);
    session->channels[IHS_SessionChannelIdStats] = IHS_SessionChannelStatsCreate(session);
    return session;
}

void IHS_SessionSetAudioCallbacks(IHS_Session *session, const IHS_StreamAudioCallbacks *callbacks, void *context) {
    IHS_BaseLock(&session->base);
    session->audioCallbacks = callbacks;
    session->audioContext = context;
    IHS_BaseUnlock(&session->base);
}

void IHS_SessionSetVideoCallbacks(IHS_Session *session, const IHS_StreamVideoCallbacks *callbacks, void *context) {
    IHS_BaseLock(&session->base);
    session->videoCallbacks = callbacks;
    session->videoContext = context;
    IHS_BaseUnlock(&session->base);
}

bool IHS_SessionConnect(IHS_Session *session) {
    IHS_BaseLock(&session->base);
    srand(time(NULL)); // NOLINT(cert-msc51-cpp)
    /* A bad RNG is enough */
    session->state.connectionId = rand() % 0xFF; // NOLINT(cert-msc50-cpp)
    IHS_BaseUnlock(&session->base);

    /* crc32c(b'Connect') */
    uint8_t body[4] = {0xc7, 0x3d, 0x8f, 0x3c};

    IHS_SessionChannel *discovery = IHS_SessionChannelFor(session, IHS_SessionChannelIdDiscovery);
    return IHS_SessionChannelSendBytes(discovery, IHS_SessionPacketTypeConnect, false, 0, body, sizeof(body), 0);
}

void IHS_SessionDisconnect(IHS_Session *session) {
    IHS_SessionChannel *discovery = IHS_SessionChannelFor(session, IHS_SessionChannelIdDiscovery);
    IHS_SessionChannelDiscoveryDisconnect(discovery);
}

void IHS_SessionRun(IHS_Session *session) {
    IHS_BaseRun(&session->base);
}

void IHS_SessionStop(IHS_Session *session) {
    IHS_BaseStop(&session->base);
}

void IHS_SessionThreadedRun(IHS_Session *session) {
    IHS_BaseThreadedRun(&session->base);
}

void IHS_SessionThreadedJoin(IHS_Session *session) {
    IHS_BaseThreadedJoin(&session->base);
}

void IHS_SessionDestroy(IHS_Session *session) {
    for (int i = 0; i < session->numChannels; ++i) {
        IHS_SessionChannelDestroy(session->channels[i]);
    }
    IHS_BaseFree(&session->base);
    free(session);
}

void IHS_SessionPacketInitialize(IHS_Session *session, IHS_SessionPacket *packet) {
    memset(packet, 0, sizeof(IHS_SessionPacket));
    packet->header.srcConnectionId = session->state.connectionId;
    packet->header.dstConnectionId = session->state.hostConnectionId;
    packet->header.sendTimestamp = IHS_SessionPacketTimestamp(session);
}

uint32_t IHS_SessionPacketTimestamp(IHS_Session *session) {
    uint64_t now = uv_now(session->base.loop);
    return IHS_SESSION_PACKET_TIMESTAMP_FROM_MILLIS(now);
}

bool IHS_SessionSendPacket(IHS_Session *session, const IHS_SessionPacket *packet) {
    const IHS_SessionConfig *config = &session->config;
    uint8_t *data = alloca(IHS_SessionPacketSize(packet));
    size_t dataSize = IHS_SessionPacketSerialize(packet, data);
    return IHS_BaseSend(&session->base, config->address, data, dataSize);
}

bool IHS_SessionSendControlMessage(IHS_Session *session, EStreamControlMessage type,
                                   const ProtobufCMessage *message, int32_t packetId) {
    IHS_SessionChannel *channel = IHS_SessionChannelFor(session, IHS_SessionChannelIdControl);
    return IHS_SessionChannelControlSend(channel, type, message, packetId);
}

static void SessionRecvCallback(uv_udp_t *handle, ssize_t nread, uv_buf_t buf, struct sockaddr *addr, unsigned flags) {
    if (!nread) {
        goto cleanup;
    }
    IHS_Session *session = handle->loop->data;
    IHS_SessionPacket packet;
    IHS_SessionPacketReturn ret = IHS_SessionPacketParse(&packet, (const uint8_t *) buf.base, nread);
    if (ret != IHS_SessionPacketResultOK) {
        goto cleanup;
    }
    IHS_SessionChannel *channel = IHS_SessionChannelFor(session, packet.header.channelId);
    if (!channel) {
        goto cleanup;
    }
    IHS_SessionChannelReceivedPacket(channel, &packet);
    cleanup:
    free(buf.base);
}