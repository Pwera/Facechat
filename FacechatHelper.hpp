#ifndef FACECHAT_FACECHATHELPER_HPP
#define FACECHAT_FACECHATHELPER_HPP

typedef long long int UserID;
typedef long long int ThreadID;
typedef long long int UniversalID;

class FacechatHelper {

public:
    std::vector<cpr::Pair>  addUserToGroup(std::vector<cpr::Pair> &payloadsPairs, UserID userID, ThreadID group, std::string mUserID){
        payloadsPairs.push_back(cpr::Pair("client", "mercury"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][action_type]", "ma-type:log-message"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][author]", "fbid:"+mUserID));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][thread_id]", ""));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][source]", "source:chat:web"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][source_tags][0]", "source:chat"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][log_message_type]", "log:subscribe"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][status]", "0"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][thread_fbid]", std::to_string(group)));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][log_message_data][added_participants][0]", "fbid:" + std::to_string(userID)));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][is_unread]", false));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][is_cleared]", false));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][is_forward]", false));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][is_filtered_content]", false));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][is_filtered_content_bh]", false));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][is_filtered_content_account]", false));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][is_spoof_warning]", false));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][timestamp]", std::to_string(time(NULL))));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][manual_retry_cnt]", "0"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][threading_id]", ""));
        return payloadsPairs;
    }

};


#endif //FACECHAT_FACECHATHELPER_HPP
