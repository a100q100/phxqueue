/*
Tencent is pleased to support the open source community by making
PhxRPC available.
Copyright (C) 2016 THL A29 Limited, a Tencent company.
All rights reserved.

Licensed under the BSD 3-Clause License (the "License"); you may
not use this file except in compliance with the License. You may
obtain a copy of the License at

https://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing
permissions and limitations under the License.

See the AUTHORS file for names of contributors.
*/

#include "mqtt_msg_handler.h"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>

#include "phxrpc/file/log_utils.h"
#include "phxrpc/network/socket_stream_base.h"

#include "mqtt_msg.h"


namespace phxqueue_phxrpc {

namespace mqttbroker {


using namespace std;


int MqttMessageHandler::RecvRequest(phxrpc::BaseTcpStream &socket, phxrpc::BaseRequest *&req) {
    Reset();

    try {
        string remaining_buffer;
        int ret{RecvFixedHeaderAndRemainingBuffer(socket, remaining_buffer)};
        if (0 != ret) {
            phxrpc::log(LOG_ERR, "RecvFixedHeaderAndRemainingBuffer err %d", static_cast<int>(ret));

            return ret;
        }

        if (MqttProtocol::ControlPacketType::CONNECT == control_packet_type_) {
            return GenConnect(remaining_buffer, req);
        } else if (MqttProtocol::ControlPacketType::PUBLISH == control_packet_type_) {
            return GenPublish(remaining_buffer, req);
        } else if (MqttProtocol::ControlPacketType::PUBACK == control_packet_type_) {
            return GenPuback(remaining_buffer, req);
        } else if (MqttProtocol::ControlPacketType::SUBSCRIBE == control_packet_type_) {
            return GenSubscribe(remaining_buffer, req);
        } else if (MqttProtocol::ControlPacketType::UNSUBSCRIBE == control_packet_type_) {
            return GenUnsubscribe(remaining_buffer, req);
        } else if (MqttProtocol::ControlPacketType::PINGREQ == control_packet_type_) {
            return GenPingreq(remaining_buffer, req);
        } else if (MqttProtocol::ControlPacketType::DISCONNECT == control_packet_type_) {
            return GenDisconnect(remaining_buffer, req);
        }
        phxrpc::log(LOG_ERR, "type %d not supported",
                    static_cast<int>(control_packet_type_));

        return -401;
    } catch (std::ios_base::failure e) {
        // a client disconnection is signalled by a EOF condition on the file descriptor.
        // the system considers EOF to be a state in which the file descriptor is 'readable'.
        // read returns 0 bytes read.
        // should close fd to prevent from being epolled again.
        phxrpc::log(LOG_ERR, "stream err %d %s", e.code().value(), e.what());

        return -103;
    }

    return -1;
}

int MqttMessageHandler::RecvResponse(phxrpc::BaseTcpStream &socket, phxrpc::BaseResponse *&resp) {
    try {
        string remaining_buffer;
        int ret{RecvFixedHeaderAndRemainingBuffer(socket, remaining_buffer)};
        if (0 != ret) {
            phxrpc::log(LOG_ERR, "RecvFixedHeaderAndRemainingBuffer err %d", static_cast<int>(ret));

            return ret;
        }

        if (MqttProtocol::ControlPacketType::CONNACK == control_packet_type_) {
            return GenConnack(remaining_buffer, resp);
        } else if (MqttProtocol::ControlPacketType::PUBLISH == control_packet_type_) {
            return GenPublish(remaining_buffer, resp);
        } else if (MqttProtocol::ControlPacketType::PUBACK == control_packet_type_) {
            return GenPuback(remaining_buffer, resp);
        } else if (MqttProtocol::ControlPacketType::SUBSCRIBE == control_packet_type_) {
            return GenSuback(remaining_buffer, resp);
        } else if (MqttProtocol::ControlPacketType::UNSUBSCRIBE == control_packet_type_) {
            return GenUnsuback(remaining_buffer, resp);
        } else if (MqttProtocol::ControlPacketType::PINGREQ == control_packet_type_) {
            return GenPingresp(remaining_buffer, resp);
        }
        phxrpc::log(LOG_ERR, "type %d not supported",
                    static_cast<int>(control_packet_type_));

        return -401;
    } catch (std::ios_base::failure e) {
        // a client disconnection is signalled by a EOF condition on the file descriptor.
        // the system considers EOF to be a state in which the file descriptor is 'readable'.
        // read returns 0 bytes read.
        // should close fd to prevent from being epolled again.
        phxrpc::log(LOG_ERR, "stream err %d %s", e.code().value(), e.what());

        return -103;
    }

    return -1;
}


int MqttMessageHandler::GenRequest(phxrpc::BaseRequest *&req) {
    req = nullptr;

    return -101;
}

int MqttMessageHandler::GenResponse(phxrpc::BaseResponse *&resp) {
    resp = req_->GenResponse();

    return 0;
}


bool MqttMessageHandler::keep_alive() const {
    return true;
}


int MqttMessageHandler::GenConnect(const string &remaining_buffer,
                                   phxrpc::BaseRequest *&req) {
    MqttConnect *connect{new MqttConnect};
    connect->set_remaining_length(remaining_buffer.size());
    req_ = req = connect;
    istringstream ss(remaining_buffer);
    return connect->RecvRemaining(ss);
}

int MqttMessageHandler::GenConnack(const string &remaining_buffer,
                                   phxrpc::BaseResponse *&resp) {
    MqttConnack *connack{new MqttConnack};
    connack->set_remaining_length(remaining_buffer.size());
    resp = connack;
    istringstream ss(remaining_buffer);
    return connack->RecvRemaining(ss);
}

int MqttMessageHandler::GenPublish(const string &remaining_buffer,
                                   phxrpc::BaseRequest *&req) {
    MqttPublish *publish{new MqttPublish};
    publish->SetFlags(flags_ & 0x0F);
    publish->set_remaining_length(remaining_buffer.size());
    req_ = req = publish;
    istringstream ss(remaining_buffer);
    return publish->RecvRemaining(ss);
}

int MqttMessageHandler::GenPublish(const string &remaining_buffer,
                                   phxrpc::BaseResponse *&resp) {
    MqttPublish *publish{new MqttPublish};
    publish->SetFlags(flags_ & 0x0F);
    publish->set_remaining_length(remaining_buffer.size());
    resp = publish;
    istringstream ss(remaining_buffer);
    return publish->RecvRemaining(ss);
}

int MqttMessageHandler::GenPuback(const string &remaining_buffer,
                                  phxrpc::BaseRequest *&req) {
    MqttPuback *puback{new MqttPuback};
    puback->set_remaining_length(remaining_buffer.size());
    req_ = req = puback;
    istringstream ss(remaining_buffer);
    return puback->RecvRemaining(ss);
}

int MqttMessageHandler::GenPuback(const string &remaining_buffer,
                                  phxrpc::BaseResponse *&resp) {
    MqttPuback *puback{new MqttPuback};
    puback->set_remaining_length(remaining_buffer.size());
    resp = puback;
    istringstream ss(remaining_buffer);
    return puback->RecvRemaining(ss);
}

int MqttMessageHandler::GenSubscribe(const string &remaining_buffer,
                                     phxrpc::BaseRequest *&req) {
    MqttSubscribe *subscribe{new MqttSubscribe};
    subscribe->set_remaining_length(remaining_buffer.size());
    req_ = req = subscribe;
    istringstream ss(remaining_buffer);
    return subscribe->RecvRemaining(ss);
}

int MqttMessageHandler::GenSuback(const string &remaining_buffer,
                                  phxrpc::BaseResponse *&resp) {
    MqttSuback *suback{new MqttSuback};
    suback->set_remaining_length(remaining_buffer.size());
    resp = suback;
    istringstream ss(remaining_buffer);
    return suback->RecvRemaining(ss);
}

int MqttMessageHandler::GenUnsubscribe(const string &remaining_buffer,
                                       phxrpc::BaseRequest *&req) {
    MqttUnsubscribe *unsubscribe{new MqttUnsubscribe};
    unsubscribe->set_remaining_length(remaining_buffer.size());
    req_ = req = unsubscribe;
    istringstream ss(remaining_buffer);
    return unsubscribe->RecvRemaining(ss);
}

int MqttMessageHandler::GenUnsuback(const string &remaining_buffer,
                                    phxrpc::BaseResponse *&resp) {
    MqttUnsuback *unsuback{new MqttUnsuback};
    unsuback->set_remaining_length(remaining_buffer.size());
    resp = unsuback;
    istringstream ss(remaining_buffer);
    return unsuback->RecvRemaining(ss);
}

int MqttMessageHandler::GenPingreq(const string &remaining_buffer,
                                   phxrpc::BaseRequest *&req) {
    MqttPingreq *pingreq{new MqttPingreq};
    pingreq->set_remaining_length(remaining_buffer.size());
    req_ = req = pingreq;
    istringstream ss(remaining_buffer);
    return pingreq->RecvRemaining(ss);
}

int MqttMessageHandler::GenPingresp(const string &remaining_buffer,
                                    phxrpc::BaseResponse *&resp) {
    MqttPingresp *pingresp{new MqttPingresp};
    pingresp->set_remaining_length(remaining_buffer.size());
    resp = pingresp;
    istringstream ss(remaining_buffer);
    return pingresp->RecvRemaining(ss);
}

int MqttMessageHandler::GenDisconnect(const string &remaining_buffer,
                                      phxrpc::BaseRequest *&req) {
    MqttDisconnect *disconnect{new MqttDisconnect};
    disconnect->set_remaining_length(remaining_buffer.size());
    req_ = req = disconnect;
    istringstream ss(remaining_buffer);
    return disconnect->RecvRemaining(ss);
}

int MqttMessageHandler::RecvRemainingLength(phxrpc::BaseTcpStream &in_stream,
                                            int &remaining_length) {
    uint32_t temp_remaining_length{0};

    char temp{0x0};
    in_stream.get(temp);
    temp_remaining_length = (static_cast<uint8_t>(temp) & 0x7f);

    if (!(static_cast<uint8_t>(temp) & 0x80)) {
        remaining_length = temp_remaining_length;

        return 0;
    }

    temp = 0x0;
    in_stream.get(temp);
    temp_remaining_length |= (static_cast<uint8_t>(temp) & 0x7f) << 7;
    if (!(static_cast<uint8_t>(temp) & 0x80)) {
        remaining_length = temp_remaining_length;

        return 0;
    }

    temp = 0x0;
    in_stream.get(temp);
    temp_remaining_length |= (static_cast<uint8_t>(temp) & 0x7f) << 14;
    if (!(static_cast<uint8_t>(temp) & 0x80)) {
        remaining_length = temp_remaining_length;

        return 0;
    }

    temp = 0x0;
    in_stream.get(temp);
    temp_remaining_length |= (static_cast<uint8_t>(temp) & 0x7f) << 21;

    remaining_length = temp_remaining_length;

    return 0;
}

int MqttMessageHandler::RecvFixedHeaderAndRemainingBuffer(
        phxrpc::BaseTcpStream &in_stream, string &remaining_buffer) {
    char fixed_header_char{0x0};
    int ret{MqttProtocol::RecvChar(in_stream, fixed_header_char)};
    if (0 != ret) {
        phxrpc::log(LOG_ERR, "RecvChar err %d", static_cast<int>(ret));

        return ret;
    }

    DecodeFixedHeader(static_cast<uint8_t>(fixed_header_char));

    phxrpc::log(LOG_DEBUG, "RecvChar type %d fixed_header %x",
                static_cast<int>(control_packet_type_),
                static_cast<uint8_t>(fixed_header_char));

    int remaining_length{0};
    ret = RecvRemainingLength(in_stream, remaining_length);

    if (0 != ret) {
        phxrpc::log(LOG_ERR, "RecvRemainingLength err %d", static_cast<int>(ret));

        return ret;
    }

    remaining_buffer.resize(remaining_length);
    ret = MqttProtocol::RecvChars(in_stream, &remaining_buffer[0], remaining_length);
    if (0 != ret) {
        phxrpc::log(LOG_ERR, "RecvChars err %d", static_cast<int>(ret));

        return ret;
    }

    return 0;
}

void MqttMessageHandler::Reset() {
    control_packet_type_ = MqttProtocol::ControlPacketType::FAKE_NONE;
    flags_ = 0x00;
}

void MqttMessageHandler::DecodeFixedHeader(const uint8_t fixed_header_byte) {
    flags_ = fixed_header_byte & 0x0F;

    uint8_t temp{fixed_header_byte};
    temp >>= 4;
    temp &= 0x0f;
    // must convert to unsigned first
    control_packet_type_ = static_cast<MqttProtocol::ControlPacketType>(temp);
}


}  // namespace mqttbroker

}  // namespace phxqueue_phxrpc
