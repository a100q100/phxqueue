/* mqttbroker_service_impl.h

 Generated by phxrpc_pb2service from mqttbroker.proto

*/

#pragma once

#include "phxrpc/network.h"

#include "phxqueue/comm.h"

#include "mqttbroker.pb.h"
#include "phxrpc_mqttbroker_service.h"
#include "server_mgr.h"


class MqttBrokerServerConfig;
class MqttPacketIdMgr;
class MqttSession;
class MqttSessionMgr;

typedef struct tagServiceArgs {
    MqttBrokerServerConfig *config;
    ServerMgr *server_mgr;
    MqttSessionMgr *mqtt_session_mgr;
    MqttPacketIdMgr *mqtt_packet_id_mgr;
} ServiceArgs_t;

class MqttBrokerServiceImpl : public MqttBrokerService {
  public:
    MqttBrokerServiceImpl(ServiceArgs_t &app_args,
            phxrpc::UThreadEpollScheduler *worker_uthread_scheduler,
            phxrpc::DataFlowArgs *data_flow_args);
    virtual ~MqttBrokerServiceImpl() override;

    virtual int PHXEcho(const google::protobuf::StringValue &req,
                        google::protobuf::StringValue *resp) override;
    virtual int PhxHttpPublish(const phxqueue_phxrpc::mqttbroker::HttpPublishPb &req,
                               phxqueue_phxrpc::mqttbroker::HttpPubackPb *resp) override;
    virtual int PhxMqttConnect(const phxqueue_phxrpc::mqttbroker::MqttConnectPb &req,
                               phxqueue_phxrpc::mqttbroker::MqttConnackPb *resp) override;
    virtual int PhxMqttPublish(const phxqueue_phxrpc::mqttbroker::MqttPublishPb &req,
                               google::protobuf::Empty *resp) override;
    virtual int PhxMqttPuback(const phxqueue_phxrpc::mqttbroker::MqttPubackPb &req,
                              google::protobuf::Empty *resp) override;
    virtual int PhxMqttPubrec(const phxqueue_phxrpc::mqttbroker::MqttPubrecPb &req,
                              google::protobuf::Empty *resp) override;
    virtual int PhxMqttPubrel(const phxqueue_phxrpc::mqttbroker::MqttPubrelPb &req,
                              google::protobuf::Empty *resp) override;
    virtual int PhxMqttPubcomp(const phxqueue_phxrpc::mqttbroker::MqttPubcompPb &req,
                               google::protobuf::Empty *resp) override;
    virtual int PhxMqttSubscribe(const phxqueue_phxrpc::mqttbroker::MqttSubscribePb &req,
                                 phxqueue_phxrpc::mqttbroker::MqttSubackPb *resp) override;
    virtual int PhxMqttUnsubscribe(const phxqueue_phxrpc::mqttbroker::MqttUnsubscribePb &req,
                                   phxqueue_phxrpc::mqttbroker::MqttUnsubackPb *resp) override;
    virtual int PhxMqttPing(const phxqueue_phxrpc::mqttbroker::MqttPingreqPb &req,
                            phxqueue_phxrpc::mqttbroker::MqttPingrespPb *resp) override;
    virtual int PhxMqttDisconnect(const phxqueue_phxrpc::mqttbroker::MqttDisconnectPb &req,
                                  google::protobuf::Empty *resp) override;

    phxqueue::comm::RetCode CheckSession(MqttSession *&mqtt_session);
    phxqueue::comm::RetCode FinishSession();
    phxqueue::comm::RetCode
    FinishRemoteSession(const std::string &client_id,
                        const phxqueue_phxrpc::mqttbroker::SessionPb &session_pb);
    phxqueue::comm::RetCode
    EnqueueMessage(const phxqueue_phxrpc::mqttbroker::HttpPublishPb &message);

  private:
    phxqueue::comm::RetCode GetStringRemote(const std::string &prefix, const std::string &key,
                                            uint64_t &version, std::string &value);

    phxqueue::comm::RetCode SetStringRemote(const std::string &prefix, const std::string &key,
                                            const uint64_t version, const std::string &value);

    phxqueue::comm::RetCode DeleteStringRemote(const std::string &prefix, const std::string &key,
                                               const uint64_t version);

    phxqueue::comm::RetCode LockRemote(const std::string &lock_key,
                                       const std::string &client_id, const uint64_t lease_time);

    phxqueue::comm::RetCode GetSessionAndClientIdBySessionIdRemote(const uint64_t session_id,
            std::string &client_id, phxqueue_phxrpc::mqttbroker::SessionPb &session_pb);

    phxqueue::comm::RetCode GetSessionByClientIdRemote(const std::string &client_id,
            uint64_t &version, phxqueue_phxrpc::mqttbroker::SessionPb &session_pb);

    phxqueue::comm::RetCode SetSessionByClientIdRemote(const std::string &client_id,
            const uint64_t version, phxqueue_phxrpc::mqttbroker::SessionPb const &session_pb);

    phxqueue::comm::RetCode DeleteSessionByClientIdRemote(const std::string &client_id,
                                                          const uint64_t version);

    phxqueue::comm::RetCode
    GetLockInfo(const phxqueue::comm::proto::GetLockInfoRequest &req,
                phxqueue::comm::proto::GetLockInfoResponse &resp);
    phxqueue::comm::RetCode
    AcquireLock(const phxqueue::comm::proto::AcquireLockRequest &req,
                phxqueue::comm::proto::AcquireLockResponse &resp);
    phxqueue::comm::RetCode
    GetString(const phxqueue::comm::proto::GetStringRequest &req,
              phxqueue::comm::proto::GetStringResponse &resp);
    phxqueue::comm::RetCode
    SetString(const phxqueue::comm::proto::SetStringRequest &req,
              phxqueue::comm::proto::SetStringResponse &resp);
    phxqueue::comm::RetCode
    DeleteString(const phxqueue::comm::proto::DeleteStringRequest &req,
                 phxqueue::comm::proto::DeleteStringResponse &resp);
    phxqueue::comm::RetCode GetTopicIDAndLockID(int &topic_id, int &lock_id);

    ServiceArgs_t &args_;
    phxrpc::UThreadEpollScheduler *worker_uthread_scheduler_{nullptr};
    phxrpc::DataFlowArgs *data_flow_args_{nullptr};
};

