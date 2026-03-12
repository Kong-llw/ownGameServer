#pragma once

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

#include "core/Types.h"

namespace Game {

enum class GroupResult {
    OK = 0,
    FULL,
    ALREADY_IN_GROUP,
    NOT_IN_GROUP,
    INVALID_CAPACITY,
};

struct GroupInfo {
    GroupId group_id{};
    std::size_t capacity{0};
    std::vector<UserId> members;
};

class Group {
public:
    Group() = default;
    explicit Group(GroupInfo info) : info_(std::move(info)) {}

    const GroupInfo& GetInfo() const { return info_; }
    GroupInfo& GetInfo() { return info_; }

    GroupId GetId() const { return info_.group_id; }
    void SetId(GroupId id) { info_.group_id = id; }

    std::size_t GetCapacity() const { return info_.capacity; }
    GroupResult SetCapacity(std::size_t new_capacity) {
        if (new_capacity < info_.members.size()) {
            return GroupResult::INVALID_CAPACITY;
        }
        info_.capacity = new_capacity;
        return GroupResult::OK;
    }

    std::size_t GetMemberCount() const { return info_.members.size(); }
    const std::vector<UserId>& GetMembers() const { return info_.members; }

    bool Contains(UserId user_id) const {
        return std::find(info_.members.begin(), info_.members.end(), user_id) != info_.members.end();
    }

    GroupResult AddMember(UserId user_id) {
        if (Contains(user_id)) {
            return GroupResult::ALREADY_IN_GROUP;
        }
        if (info_.members.size() >= info_.capacity) {
            return GroupResult::FULL;
        }
        info_.members.push_back(user_id);
        return GroupResult::OK;
    }

    GroupResult RemoveMember(UserId user_id) {
        auto it = std::find(info_.members.begin(), info_.members.end(), user_id);
        if (it == info_.members.end()) {
            return GroupResult::NOT_IN_GROUP;
        }
        info_.members.erase(it);
        return GroupResult::OK;
    }

private:
    GroupInfo info_;
};

} // namespace Game
