#include "business/IMsgHandler.hpp"
#include "business/GameRoom/GameRoomManager.hpp"
namespace Game{
class RoomReqHandler : public IMsgHandler {
public:
    using RoomJoinCallback = std::function<void(RoomJoinInfo)>;
    using RoomLeaveCallback = std::function<void(RoomJoinInfo)>;

    RoomReqHandler(asio::strand<asio::any_io_executor> strand, std::shared_ptr<GameRoomManager> game_room_manager) 
    : strand_(std::move(strand)), game_room_manager_(std::move(game_room_manager)) {}
    RoomReqHandler() = delete;
    ~RoomReqHandler() = default;
    //此函数必须为异步实现, 避免阻塞Router
    bool HandleDecodedMsg(const std::shared_ptr<Network::MsgPack>& msg);
    bool Authentication(const std::shared_ptr<Network::MsgPack>& msg);
    void SetGateway(std::shared_ptr<Network::IBusinessMsgGateway> gateway) { gateway_ = std::move(gateway); }
    
    bool HandleRoomCreateRequest(const std::shared_ptr<Network::MsgPack>&& msg);
    bool HandleRoomJoinRequest(const std::shared_ptr<Network::MsgPack>&& msg);
    bool HandleRoomLeaveRequest(const std::shared_ptr<Network::MsgPack>&& msg);
    bool HandleRoomListRequest(const std::shared_ptr<Network::MsgPack>&& msg);

    bool RegRoomJoinCallback(RoomJoinCallback cb) {
        join_room_callbacks_.push_back(std::move(cb));
        return true;
    }
    bool RegRoomLeaveCallback(RoomLeaveCallback cb) {
        leave_room_callbacks_.push_back(std::move(cb));
        return true;
    }
private:
    asio::strand<asio::any_io_executor> strand_; //保证同一时间只有一个请求在处理, 避免并发问题
    std::shared_ptr<GameRoomManager> game_room_manager_;
    std::shared_ptr<Network::IBusinessMsgGateway> gateway_; //用于转发消息
    std::vector<RoomJoinCallback> join_room_callbacks_;
    std::vector<RoomLeaveCallback> leave_room_callbacks_;
};

}