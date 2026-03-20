#include "JsonTranslator.hpp"
#include <iostream>

using nlohmann::json;

std::string JSONTranslator::createGameCommandJSON(GameCmdType cmdType, const std::string& cmdData) {
    return R"({"cmd_type":)" + std::to_string(static_cast<uint8_t>(cmdType)) + R"(,"cmd_data":)" + cmdData + R"(})";
}
//所有使用到这个转义功能的，都没有做好避免拷贝的处理，会进行一次string->vector<byte>的拷贝。
//识别json并转化为BattleCmd结构，供GameRoom使用
BattleCmd JSONTranslator::ParseCommand(const std::string& jsonStr) {
    BattleCmd cmd{};
    try {
        json root = json::parse(jsonStr);

        if(!root.contains("cmd_type")){
            std::cerr << "ParseCommand: missing cmd_type" << std::endl;
            return cmd;
        }

        uint8_t cmd_type = 0;
        try{
            cmd_type = static_cast<uint8_t>(root.at("cmd_type").get<int>());
        } catch(...) {
            std::cerr << "ParseCommand: invalid cmd_type format" << std::endl;
            return cmd;
        }

        std::string cmd_data_str;
        if(root.contains("cmd_data")){
            // cmd_data may be object or string; keep its JSON text
            if(root["cmd_data"].is_string()){
                cmd_data_str = root["cmd_data"].get<std::string>();
            } else {
                cmd_data_str = root["cmd_data"].dump();
            }
        }

        JsonCommand pkt;
        pkt.cmd_type = cmd_type;
        pkt.cmd_data = cmd_data_str;

        switch(static_cast<GameCmdType>(pkt.cmd_type)){
            case GAMECMD_MOVE: {
                try{
                    json d = json::parse(pkt.cmd_data);
                    JsonMoveCommand mv{};
                    mv.source_id = d.value("source_id", 0u);
                    mv.dest_id = d.value("dest_id", 0u);
                    mv.value = d.value("value", 0u);
                    std::cout << "Parsed MOVE: src=" << mv.source_id << " dst=" << mv.dest_id << " val=" << mv.value << std::endl;
                    
                    cmd.player_id = 0;
                    cmd.type = BattleCmdType::Move;
                    cmd.param1 = mv.source_id;
                    cmd.param2 = mv.dest_id;
                } catch(const std::exception& e){
                    std::cerr << "ParseCommand: invalid MOVE data: " << e.what() << std::endl;
                }
                break;
            }
            case GAMECMD_ATTACK: {
                try{
                    json d = json::parse(pkt.cmd_data);
                    JsonAttackCommand at{};
                    at.source_id = d.value("source_id", 0u);
                    at.dest_id = d.value("dest_id", 0u);
                    at.source_value = d.value("source_value", 0u);
                    at.dest_value = d.value("dest_value", 0u);
                    std::cout << "Parsed ATTACK: src=" << at.source_id << " dst=" << at.dest_id << " sVal=" << at.source_value << " dVal=" << at.dest_value << std::endl;
                    
                    cmd.player_id = 0;
                    cmd.type = BattleCmdType::Attack;
                    cmd.param1 = at.source_id;
                    cmd.param2 = at.dest_id;
                } catch(const std::exception& e){
                    std::cerr << "ParseCommand: invalid ATTACK data: " << e.what() << std::endl;
                }
                break;
            }
            case GAMECMD_GROW: {
                try{
                    json d = json::parse(pkt.cmd_data);
                    JsonGrowCommand gr{};
                    gr.target_tile_id = d.value("target_tile_id", 0u);
                    gr.grow_value = d.value("grow_value", 0u);
                    std::cout << "Parsed GROW: tile=" << gr.target_tile_id << " grow=" << gr.grow_value << std::endl;
                    
                    cmd.player_id = 0;
                    cmd.type = BattleCmdType::Grow;
                    cmd.param1 = gr.target_tile_id;
                    cmd.param2 = gr.grow_value; 
                } catch(const std::exception& e){
                    std::cerr << "ParseCommand: invalid GROW data: " << e.what() << std::endl;
                }
                break;
            }
            default:
                std::cout << "ParseCommand: unhandled cmd_type=" << int(pkt.cmd_type) << " data=" << pkt.cmd_data << std::endl;
        }

    } catch(const json::parse_error& e) {
        std::cerr << "ParseCommand: JSON parse error: " << e.what() << std::endl;
    } catch(const std::exception& e){
        std::cerr << "ParseCommand: exception: " << e.what() << std::endl;
    }

    return cmd;
}

std::string JSONTranslator::serializeRoomList(const std::vector<RoomInListInfo>& rooms) {
    json root;
    root["cmd_type"] = GAMEDATA_ROOMLIST;
    root["rooms"] = json::array();
    auto& roomArray = root["rooms"];
    json r;
    for(const auto& room : rooms){
        r.clear();
        r["room_code"] = room.room_code;
        r["room_name"] = room.room_name;
        r["capacity"] = room.capacity;
        r["player_count"] = room.player_count;
        r["state"] = static_cast<uint8_t>(room.state);
        roomArray.emplace_back(std::move(r));
    }
    return root.dump();
}
std::string JSONTranslator::serializeRoomInfo(const GameRoomInfo& info){
    json root;
    root["cmd_type"] = GAMEDATA_PLAYERINROOM;
    root["players"] = json::array();
    auto& playerArray = root["players"];
    json p;
    for(const auto& player : info.players){
        p.clear();
        p["user_id"] = player.b_info.user_id;
        p["user_name"] = std::string(player.b_info.user_name);
        p["seat_index"] = player.seat_index;
        p["color"] = player.color;
        p["position"] = player.position;
        playerArray.emplace_back(std::move(p));
    }
    root["state"] = static_cast<uint8_t>(info.state);
    root["room_code"] = info.room_code;
    root["room_name"] = info.room_name;
    root["password"] = info.password;
    root["capacity"] = info.capacity;
    root["player_count"] = info.player_count;
    root["selected_map_path"] = info.selected_map_path;
    root["owner_id"] = info.owner_id;
    root["seat_index"] = info.player_count -1;//MARK 看情况改逻辑，如果座位号需要特殊维护的话
    return root.dump();
}

std::string JSONTranslator::serializeMatchInfo(const MatchInfo& info){
    json root;
    root["cmd_type"] = GAMEDATA_PLAYERINROOM;
    root["players"] = json::array();
    auto& playerArray = root["players"];
    json p;
    for(const auto& player : info.players){
        p.clear();
        p["user_id"] = player.b_info.user_id;
        p["user_name"] = std::string(player.b_info.user_name);
        p["seat_index"] = player.seat_index;
        p["color"] = player.color;
        p["position"] = player.position;
        playerArray.emplace_back(std::move(p));
    }
    root["selected_map_path"] = info.selected_map_path;
    root["OwnerID"] = info.owner_id;
    return root.dump();
}


std::string JSONTranslator::serializeCreateRoomResult(const std::string& room_code, MsgProto::RoomReqResult res) {
    json root;
    root["room_code"] = room_code;
    root["result"] = static_cast<int>(res);
    return root.dump();
}