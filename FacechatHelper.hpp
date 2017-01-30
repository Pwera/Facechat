#ifndef FACECHAT_FACECHATHELPER_HPP
#define FACECHAT_FACECHATHELPER_HPP

typedef long long int UserID;
typedef long long int ThreadID;
typedef long long int UniversalID;

class FacechatHelper {

public:
    void setGroupTitle(std::vector<cpr::Pair> &payloadsPairs, UniversalID id, std::string title, std::string mUserID) {
        payloadsPairs.push_back(cpr::Pair("client", "mercury"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][action_type]", "ma-type:log-message"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][author]", "fbid:" + mUserID));
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

    void addUserToGroup(std::vector<cpr::Pair> &payloadsPairs, UserID userID, ThreadID group, std::string mUserID) {
        payloadsPairs.push_back(cpr::Pair("client", "mercury"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][action_type]", "ma-type:log-message"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][author]", "fbid:" + mUserID));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][thread_id]", ""));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][source]", "source:chat:web"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][source_tags][0]", "source:chat"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][log_message_type]", "log:subscribe"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][status]", "0"));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][thread_fbid]", std::to_string(group)));
        payloadsPairs.push_back(cpr::Pair("message_batch[0][log_message_data][added_participants][0]",
                                          "fbid:" + std::to_string(userID)));
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

    void loadMorePosts(std::vector<cpr::Pair> &payloadsPairs, std::string mUserID) {
        payloadsPairs.push_back(cpr::Pair("page_id", mUserID));
        payloadsPairs.push_back(cpr::Pair("cursor",
                                          "{\"timeline_cursor\":\"timeline_unit:1:00000000001485276972:04611686018427387904:09223372036854775807:04611686018427387904\",\"timeline_section_cursor\":{},\"has_next_page\":true}"));
        payloadsPairs.push_back(cpr::Pair("surface", "www_pages_home"));
        payloadsPairs.push_back(cpr::Pair("unit_count", "8"));
        payloadsPairs.push_back(cpr::Pair("dpr", "1.5"));
        payloadsPairs.push_back(cpr::Pair("__pc", "PHASED:DEFAULT"));
        payloadsPairs.push_back(cpr::Pair("__af", "i0"));
        payloadsPairs.push_back(cpr::Pair("__be", "-1"));
        payloadsPairs.push_back(cpr::Pair("__dyn",
                                          "aihoFeyfyGmagngDxyG8EiolzkqbxqbAKGiBAy8Z9LFwxBxC9V8CdwIhE98nwgUaqwHUR7yUJi28y4EnFeex3BKuEjKewzWxaFQ12VVojxCVEiHWCDxi5-uifz8lUlwkEG9J7By8K48hxGbwBxq69LZ1uJ12VovGi5qh98FoKEWdxyayoO9GVtqgmx2ii49um"));

    }

    void updateStatus(std::vector<cpr::Pair> &payloadsPairs, std::string &composer_id) {
        std::string message = "https://www.facebook.com/permalink.php?story_fbid=149737425529324&id=100014792015409";
        std::string composer_session_id = "4ffe58b9-1adb-4975-8ab2-2982ca016c4c";
        payloadsPairs.push_back(cpr::Pair("attachment[params][0]", "1225650124162855"));
        payloadsPairs.push_back(cpr::Pair("attachment[type]", "11"));
        payloadsPairs.push_back(cpr::Pair("attachment[reshare_original_post]", "false"));
        payloadsPairs.push_back(cpr::Pair("backdated_date[year]", ""));
        payloadsPairs.push_back(cpr::Pair("backdated_date[month]", ""));
        payloadsPairs.push_back(cpr::Pair("backdated_date[day]", ""));
        payloadsPairs.push_back(cpr::Pair("backdated_date[hour]", ""));
        payloadsPairs.push_back(cpr::Pair("backdated_date[minute]", ""));
        payloadsPairs.push_back(cpr::Pair("boosted_post_config", ""));
        payloadsPairs.push_back(cpr::Pair("composer_entry_time", "13"));
        payloadsPairs.push_back(cpr::Pair("composer_session_id", composer_session_id));
        payloadsPairs.push_back(cpr::Pair("composer_session_duration", "20"));
        payloadsPairs.push_back(cpr::Pair("composer_source_surface", "timeline"));
        payloadsPairs.push_back(cpr::Pair("composertags_city", ""));
        payloadsPairs.push_back(cpr::Pair("composertags_place", ""));
        payloadsPairs.push_back(cpr::Pair("direct_share_status", "0"));
        payloadsPairs.push_back(cpr::Pair("feed_topics", ""));
        payloadsPairs.push_back(cpr::Pair("hide_object_attachment", "true"));
        payloadsPairs.push_back(cpr::Pair("is_explicit_place", "false"));
        payloadsPairs.push_back(cpr::Pair("is_markdown", "false"));
        payloadsPairs.push_back(cpr::Pair("is_q_and_a", "false"));
        payloadsPairs.push_back(cpr::Pair("is_profile_badge_post", "false"));
        payloadsPairs.push_back(cpr::Pair("multilingual_specified_lang", ""));
        payloadsPairs.push_back(cpr::Pair("num_keystrokes", "67"));
        payloadsPairs.push_back(cpr::Pair("num_pastes", "1"));
        payloadsPairs.push_back(cpr::Pair("post_surfaces_blacklist", ""));
        payloadsPairs.push_back(cpr::Pair("privacyx", "300645083384735"));
        payloadsPairs.push_back(cpr::Pair("prompt_id", ""));
        payloadsPairs.push_back(cpr::Pair("prompt_tracking_string", ""));
        payloadsPairs.push_back(cpr::Pair("ref", "timeline"));
        payloadsPairs.push_back(cpr::Pair("target_type", "feed"));
        payloadsPairs.push_back(cpr::Pair("xhpc_message", message));
        payloadsPairs.push_back(cpr::Pair("xhpc_message_text", message));
        payloadsPairs.push_back(cpr::Pair("is_forced_reshare_of_post", "false"));
        payloadsPairs.push_back(cpr::Pair("xc_share_params", "[1225650124162855]"));
        payloadsPairs.push_back(cpr::Pair("xc_share_target_type", "11"));
        payloadsPairs.push_back(cpr::Pair("is_react", "true"));
        payloadsPairs.push_back(cpr::Pair("xhpc_composerid", composer_id));
        payloadsPairs.push_back(cpr::Pair("xhpc_targetid", "100014792015409"));
        payloadsPairs.push_back(cpr::Pair("xhpc_context", "profile"));
        payloadsPairs.push_back(cpr::Pair("xhpc_ismeta", "1"));
        payloadsPairs.push_back(cpr::Pair("xhpc_timeline", "true"));
        payloadsPairs.push_back(cpr::Pair("xhpc_finch", "false"));
        payloadsPairs.push_back(cpr::Pair("xhpc_socialplugin", "false"));
        payloadsPairs.push_back(cpr::Pair("xhpc_topicfeedid", ""));
        payloadsPairs.push_back(cpr::Pair("xhpc_origintopicfeedid", ""));
        payloadsPairs.push_back(cpr::Pair("xhpc_modal_composer", "false"));
        payloadsPairs.push_back(cpr::Pair("xhpc_aggregated_story_composer", "false"));
        payloadsPairs.push_back(cpr::Pair("xhpc_publish_type", "1"));
        payloadsPairs.push_back(cpr::Pair("xhpc_fundraiser_page", "false"));
//        payloadsPairs.push_back(cpr::Pair("__user", "100014792015409"));
//        payloadsPairs.push_back(cpr::Pair("__a", 1));
        payloadsPairs.push_back(cpr::Pair("__pc", "PHASED:DEFAULT"));
        payloadsPairs.push_back(cpr::Pair("__af", "i0"));
        payloadsPairs.push_back(cpr::Pair("__be", "-1"));
        payloadsPairs.push_back(cpr::Pair("__dyn",
                                          "aihoFeyfyGmagngDxyG8EiolzkqbxqbAKGiBAy8Z9LFwxBxC9V8CdwIhE98nwgUaqwHUR7yUJi28y4EnFeex3BKuEjKewzWxaFQ12VVojxCVEiHWCDxi5-uifz8lUlwkEG9J7By8K48hxGbwBxq69LZ1uJ12VovGi5qh98FoKEWdxyayoO9GVtqgmx2ii49um"));


    }

    void reactComposer(std::vector<cpr::Pair> &payloadsPairs, std::string &composer_id) {
        std::string message = "https://www.facebook.com/permalink.php?story_fbid=149737425529324&id=100014792015409";
        payloadsPairs.push_back(cpr::Pair("composer_id", composer_id));
        payloadsPairs.push_back(cpr::Pair("target_id", "100014792015409"));
        payloadsPairs.push_back(cpr::Pair("scrape_url", message));
        payloadsPairs.push_back(cpr::Pair("entry_point", "timeline"));
        payloadsPairs.push_back(cpr::Pair("source_attachment", "STATUS"));
        payloadsPairs.push_back(cpr::Pair("source_logging_name", "link_pasted"));
        payloadsPairs.push_back(cpr::Pair("av", "100014792015409"));
        payloadsPairs.push_back(cpr::Pair("dpr", "1.5"));
        payloadsPairs.push_back(cpr::Pair("__dyn",
                                          "aihoFeyfyGmagngDxyG8EiolzkqbxqbAKGiBAy8Z9LFwxBxC9V8CdwIhE98nwgUaqwHUR7yUJi28y4EnFeex3BKuEjKewzWxaFQ12VVojxCVEiHWCDxi5-uifz8lUlwkEG9J7By8K48hxGbwBxq69LZ1uJ12VovGi5qh98FoKEWdxyayoO9GVtqgmx2ii49um"));
        payloadsPairs.push_back(cpr::Pair("__af", "i0"));
//        payloadsPairs.push_back(cpr::Pair("__req", "b4"));
        payloadsPairs.push_back(cpr::Pair("__be", "-1"));
        payloadsPairs.push_back(cpr::Pair("__pc", "PHASED:DEFAULT"));
    }

    void logJson(cpr::Response &r, std::string message = "", bool toFile = false) {
        std::cout << "------------------------------------------------------\n";
        std::cout << message << " statusCode: " << r.status_code << " " << r.url << " " << " error: " << r.error.message
                  <<
                  std::endl;

        std::cout << "------------------------------------------------------end of " << message << "\n";
        if (toFile) {
            std::ofstream write;
            write.open(message.append(".txt"));
            write << r.text;
            write.close();
        }
    }

    void logPayloads(std::vector<cpr::Pair> &payloadsPairs) {
        for (int i = 0; i <payloadsPairs.size() ; ++i) {
            std::cout<<std::to_string(i) << ": " <<payloadsPairs[i].key << " "<< payloadsPairs[i].value<<std::endl;
        }

    }

    std::vector<std::string> parseResponseForUserPosts(cpr::Response r) {
        std::string sub = "/posts/";
        std::vector<std::string> postsPositions;
        size_t pos = r.text.find(sub, 0);
        while (pos != std::string::npos) {
            try {
                pos = r.text.find(sub, pos + 1);
                std::string subStr = r.text.substr(pos, sub.size() + 16);
                postsPositions.push_back("https://www.facebook.com/slawomirmentzen" + subStr);
                //https://www.facebook.com/slawomirmentzen/posts/1274753685919165
                std::cout << "Post: " << subStr << std::endl;
            } catch (std::out_of_range &exception) {
                std::cout << "Exception in graph api" << exception.what() << "\n";
            }
        }
        std::cout << "Counts of posts: " << postsPositions.size() << std::endl;

        sort(postsPositions.begin(), postsPositions.end());
        postsPositions.erase(unique(postsPositions.begin(), postsPositions.end()), postsPositions.end());

        std::cout << "Counts of posts: " << postsPositions.size() << std::endl;

        return postsPositions;
    }

};


#endif //FACECHAT_FACECHATHELPER_HPP
