/*****************************************************************
 * Description
 * Email huxiaoheigame@gmail.com
 * Created on 2022/09/22
 * Copyright (c) 2021 虎小黑
 ****************************************************************/

#include "config.h"

namespace tiger {

typedef std::list<std::pair<std::string, const YAML::Node>> AllNodes;

static void list_all_member(const YAML::Node &root,
                            const std::string &prefix,
                            AllNodes &all_nodes) {
    if (!IsValidName(prefix) || prefix.empty()) {
        TIGER_LOG_E(SYSTEM_LOG) << "[INVALID KEY: " << prefix << "]";
        return;
    }
    all_nodes.push_back(std::make_pair(prefix, root));
    if (!root.IsMap()) return;
    auto it = root.begin();
    while (it != root.end()) {
        list_all_member(it->second, prefix + "." + it->first.Scalar(), all_nodes);
        ++it;
    }
}

ConfValBase::ptr Config::LookupBase(const std::string &name) {
    auto it = S_DATAS().find(name);
    if (it == S_DATAS().end()) return nullptr;
    return it->second;
}

void Config::LoadFromYmal(const std::string &name, const std::string &ymal_path) {
    AllNodes all_nodes;
    auto root = YAML::LoadFile(ymal_path);
    list_all_member(root, name, all_nodes);
    for (auto &it : all_nodes) {
        ConfValBase::ptr var = LookupBase(it.first);
        if (!var) continue;
        if (it.second.IsScalar()) {
            var->from_string(it.second.Scalar());
        } else {
            std::stringstream ss;
            ss << it.second;
            var->from_string(ss.str());
        }
    }
}

}  // namespace tiger