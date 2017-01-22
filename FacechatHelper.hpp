#ifndef FACECHAT_FACECHATHELPER_HPP
#define FACECHAT_FACECHATHELPER_HPP

typedef long long int UserID;
typedef long long int ThreadID;
typedef long long int UniversalID;

class FacechatHelper {

public:
    void setGroupTitle(std::vector<cpr::Pair>& payloadsPairs ,UniversalID id, std::string title, std::string mUserID){
        payloadsPairs.push_back(cpr::Pair("client", "mercury"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][action_type]", "ma-type:log-message"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][author]", "fbid:"+mUserID));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][thread_id]", ""));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][source]", "source:chat:web"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][source_tags][0]", "source:chat"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][log_message_type]", "log:thread-name"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][status]", "0"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][thread_fbid]", std::to_string(id)));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][log_message_data][name]", title));
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

        std::string threadingID;
        payloadsPairs.push_back(cpr::Pair("message_batch[0][message_id]", threadingID));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][offline_threading_id]", threadingID));
    }

    void addUserToGroup(std::vector<cpr::Pair> &payloadsPairs, UserID userID, ThreadID group, std::string mUserID){
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
    }
    void logJson(cpr::Response& r, std::string message= "", bool toFile=false){
        std::cout<<"------------------------------------------------------\n";
        std::cout<<message<< " statusCode: "<<r.status_code << " " << " error: "<<r.error.message<<" text: "<<r.text<<std::endl;
        std::cout<<"------------------------------------------------------end of "<<message << "\n";
        std::ofstream write;
        write.open(message.append(".txt"));
        write<<r.text;
        write.close();

    }

};


#endif //FACECHAT_FACECHATHELPER_HPP
