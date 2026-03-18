#pragma once

#include <algorithm>
#include <cstddef>
#include <type_traits>
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

template <typename MemberT>
struct GroupInfo {
    GroupId group_id{};
    std::size_t capacity{0};
    std::vector<MemberT> members;
};

template <typename MemberT>
struct IdentityKeyExtractor {
    const MemberT& operator()(const MemberT& member) const { return member; }
};

template <typename MemberT, typename KeyExtractor = IdentityKeyExtractor<MemberT>>
class Group {
public:
    using KeyType = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<KeyExtractor>()(std::declval<const MemberT&>()))>>;

    Group() = default;
    explicit Group(GroupInfo<MemberT> info, KeyExtractor key_extractor = KeyExtractor())
        : info_(std::move(info)), key_extractor_(std::move(key_extractor)) {
        info_.members.reserve(info_.capacity);
    }

    const GroupInfo<MemberT>& GetInfo() const { return info_; }
    GroupInfo<MemberT>& GetInfo() { return info_; }

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
    const std::vector<MemberT>& GetMembers() const { return info_.members; }
    std::vector<MemberT>& GetMembers() { return info_.members; }

    bool Contains(const KeyType& member_key) const {
        return FindMember(member_key) != nullptr;
    }

    GroupResult AddMember(MemberT member) {
        if (Contains(key_extractor_(member))) {
            return GroupResult::ALREADY_IN_GROUP;
        }
        if (info_.members.size() >= info_.capacity) {
            return GroupResult::FULL;
        }
        info_.members.push_back(std::move(member));
        return GroupResult::OK;
    }

    GroupResult RemoveMember(const KeyType& member_key) {
        auto it = FindMemberIter(member_key);
        if (it == info_.members.end()) {
            return GroupResult::NOT_IN_GROUP;
        }
        info_.members.erase(it);
        return GroupResult::OK;
    }

    MemberT* FindMember(const KeyType& member_key) {
        auto it = FindMemberIter(member_key);
        if (it == info_.members.end()) {
            return nullptr;
        }
        return &(*it);
    }

    const MemberT* FindMember(const KeyType& member_key) const {
        auto it = FindMemberIter(member_key);
        if (it == info_.members.end()) {
            return nullptr;
        }
        return &(*it);
    }

    void ClearMembers() { info_.members.clear(); }

private:
    auto FindMemberIter(const KeyType& member_key) {
        return std::find_if(info_.members.begin(), info_.members.end(),
            [&](const MemberT& member) { return key_extractor_(member) == member_key; });
    }

    auto FindMemberIter(const KeyType& member_key) const {
        return std::find_if(info_.members.begin(), info_.members.end(),
            [&](const MemberT& member) { return key_extractor_(member) == member_key; });
    }

    GroupInfo<MemberT> info_;
    KeyExtractor key_extractor_{};
};

} // namespace Game
