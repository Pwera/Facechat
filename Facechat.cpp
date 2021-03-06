#include "Facechat.h"
#include <algorithm>

Facechat::Facechat() {
//    mDefaultCookie["_js_reg_fb_gate"] = login_url;
    mDefaultCookie["reg_fb_gate"] = login_url;
    mDefaultCookie["reg_fb_ref"] = login_url;
//    mDefaultCookie["datr"] = "noZkV5yIO_tft56tOn_rZlet";
//    mDefaultCookie["_js_session"] = "false";
//    mDefaultCookie["presence"] = "EDvF3EtimeF1466382654EuserFA21B00803925467A2EstateFDt2F_5bDiFA2user_3a1B03742384564A2ErF1C_5dElm2FA2user_3a1B03742384564A2Euct2F1466382291289EtrFnullEtwF1685193339EatF1466382654078G466382654617CEchFDp_5f1B00803925467F2CC";

    mPostSession.SetHeader(cpr::Header {{"User-Agent", user_agent}});
    mUpdateSession.SetHeader(cpr::Header {{"User-Agent", user_agent}});

    mPostSession.SetCookies(mDefaultCookie);
    mUpdateSession.SetCookies(mDefaultCookie);
}

Facechat::~Facechat() {
    logout();
}

int Facechat::login(std::string email, std::string password) {
    mPostSession.SetUrl(cpr::Url {login_url});
    mPostSession.SetPayload(cpr::Payload {{"email", email},
                                          {"pass",  password}});
    mUpdateSession.SetPayload(cpr::Payload {{"email", email},
                                            {"pass",  password}});
    cpr::Response r = mPostSession.Post();
//    helper.logJson(r, "login");

    int pos = r.text.find("\"USER_ID\":\"");
    if (pos != std::string::npos)
        mUserID = r.text.substr(pos + 11, r.text.find("\"", pos + 11) - (pos + 11));

    pos = r.text.find("name=\"fb_dtsg\" value=\"");
    if (pos != std::string::npos)
        mDTSG = r.text.substr(pos + 22, r.text.find("\"", pos + 22) - (pos + 22));

    pos = r.text.find("revision\":");
    if (pos != std::string::npos)
        mRevision = r.text.substr(pos + 10, r.text.find(",\"", pos + 10) - (pos + 10));

    pos = r.text.find("signa", 0);
    //std::cout << "signature id : " << pos << std::endl;
    r.text = ""; //for debug cuz the text is too big and make gdb lag

    mPostSession.SetUrl(cpr::Url({replaceAll(sticky_url, "$USER_ID", mUserID)}));
    r = mPostSession.Get();

    pos = r.text.find("\"sticky\":\"");
    if (pos != std::string::npos)
        mSticky = r.text.substr(pos + 10, r.text.find("\"", pos + 10) - (pos + 10));

    pos = r.text.find("\"pool\":\"");
    if (pos != std::string::npos)
        mPool = r.text.substr(pos + 8, r.text.find("\"", pos + 8) - (pos + 8));

    if (mUserID.size() && mDTSG.size() && mSticky.size() && mPool.size()) {
        mStopThread = false;
        mUpdateThread = std::thread(std::bind(&Facechat::update, this));
        mUpdateThread.detach();
        return 1;
    } else
        return 0;
}

void Facechat::logout() {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(logout_url);
    mPostSession.Post();

    mStopThread = true;
    mDeadMutex.lock();

    mUserID.clear();
    mDTSG.clear();
    mSticky.clear();
    mPool.clear();

    mPostSession.~Session();
    mUpdateSession.~Session();
    new(&mPostSession) cpr::Session();
    new(&mUpdateSession) cpr::Session();

    mDefaultPayloads.clear();
}

bool Facechat::pullMessage(MessagingEvent &event) {
    std::lock_guard<std::mutex> lock(mEventsMutex);
    if (mEvents.empty())
        return 0;

    event = mEvents.back();
    mEvents.pop_back();

    return 1;
}

void Facechat::update() {
    try {
        std::lock_guard<std::mutex> lock(mDeadMutex);

        mUpdateSession.SetUrl(cpr::Url{login_url});
        mUpdateSession.Post();

        while (!mStopThread) {
            std::string url = replaceAll(pull_message_url, "$USER_ID", mUserID);
            url = replaceAll(url, "$STICKY", mSticky);
            url = replaceAll(url, "$POOL", mPool);
            url = replaceAll(url, "$SEQ", seq);

            mUpdateSession.SetUrl(cpr::Url(url));
            cpr::Response r = mUpdateSession.Get();

            if (r.status_code) {
                json j = responseToJson(r);

                if (j["seq"].is_number())
                    seq = std::to_string(j["seq"].get<int>());

                j = j["ms"];

                for (int x = 0; x < j.size(); x++) {
                    MessagingEvent event;

                    auto m = j[x];

                    if (m["type"] == "typ") {
                        event.type = MessagingEvent::TYPING_STATUS;
                        event.typingStatus.from = m["from"];
                        event.typingStatus.fromMobile = (!m["from_mobile"].is_null() ? m["from_mobile"].get<bool>()
                                                                                     : false);
                        event.typingStatus.isTyping = m["st"] == "true";
                    } else if (m["type"] == "buddylist_overlay") {
                        event.type = MessagingEvent::ONLINE_STATUS;
                        json::iterator it = m["overlay"].begin();
                        std::string from = it.key();
                        event.onlineStatus.from = std::stoll(from);
                        event.onlineStatus.timestamp = it.value()["la"];
//                    event.onlineStatus.status                = OnlineStatus::stringToStatus(it.value()["p"]["status"]);
//                    event.onlineStatus.onFacechatApplication = OnlineStatus::stringToStatus(it.value()["p"]["fbAppStatus"]);
//                    event.onlineStatus.onMessenger           = OnlineStatus::stringToStatus(it.value()["p"]["messengerStatus"]);
//                    event.onlineStatus.onWeb                 = OnlineStatus::stringToStatus(it.value()["p"]["webStatus"]);
//                    event.onlineStatus.onOther               = OnlineStatus::stringToStatus(it.value()["p"]["otherStatus"]);
                    } else if ((m["type"] == "delta" && m["delta"]["class"] == "NewMessage")) {
                        event.type = MessagingEvent::MESSAGE;
//                    event.message = parseMessage(m["delta"]);
                        event.message = parseUpdatMessage(m["delta"]);

                    } else if ((m["type"] == "messaging" && m["event"] == "m_read_receipt")) {
                        event.type = MessagingEvent::READ_STATUS;
                        ReadStatus readStatus;
                        readStatus.from = m["reader"];
                        event.readStatus = readStatus;
                    } else {
//                    std::cout << m.dump(4) << std::endl;
//                    continue;
                    }

//                std::cout << std::endl;
//                std::cout << x << std::endl;
//                std::cout << m.dump(4) << std::endl;
//                std::cout << std::endl;

                    mEventsMutex.lock();
                    mEvents.push_front(event);
                    mEventsMutex.unlock();
                }
            }
        }
    } catch (std::exception &e) {
        std::cout << "Exception in update(): " << e.what() << std::endl;
    }
}

json Facechat::responseToJson(cpr::Response &response) {
    static const std::string toRemove = "for (;;);";


    if (response.text.size() >= toRemove.size()) {
        try {
            auto k = response.text.substr(toRemove.size());
            auto toReturn = json::parse(k);
            return toReturn;
        } catch (std::invalid_argument &e) {
            std::cout << "Exception, invalid_argument inside responseToJson " << e.what() << " " << std::endl;

        }

    }
    return json();
}


std::string Facechat::send(const std::vector<cpr::Pair> &data) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    payloadsPairs.push_back(cpr::Pair("__af", "i0"));
    payloadsPairs.push_back(cpr::Pair("__be", "-1"));
    payloadsPairs.push_back(cpr::Pair("__dyn",
                                      "aihoFeyfyGmagngDxyG8EiolzkqbxqbAKGiBAy8Z9LFwxBxC9V8CdwIhE98nwgUaqwHUR7yUJi28y4EnFeex3BKuEjKewzWxaFQ12VVojxCVEiHWCDxi5-uifz8lUlwkEG9J7By8K48hxGbwBxq69LZ1uJ12VovGi5qh98FoKEWdxyayoO9GVtqgmx2ii49um"));
    payloadsPairs.push_back(cpr::Pair("__pc", "PHASED:DEFAULT"));

    payloadsPairs.insert(payloadsPairs.end(), data.begin(), data.end());
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {send_message_url});
    cpr::Response r = mPostSession.Post();
    helper.logJson(r, "send");
    auto response = responseToJson(r)["payload"]["actions"][0]["message_id"];

    if (response.is_string())
        return response;
    else
        return "";
}

std::string Facechat::sendMessage(std::string message, UniversalID sendTo, bool hasAttachment, std::vector<cpr::Pair> datas) {
    datas.emplace_back("body", message);


        auto myNumber = generateMessageID2();
        datas.push_back(cpr::Pair("specific_to_list[1]", "fbid:" + mUserID));
        datas.push_back(cpr::Pair("specific_to_list[0]", "fbid:" + std::to_string(sendTo)));
        datas.push_back(cpr::Pair("other_user_fbid", std::to_string(sendTo)));
        datas.push_back(cpr::Pair("action_type", "ma-type:user-generated-message"));
        datas.push_back(cpr::Pair("ephemeral_ttl_mode", "0"));
        datas.push_back(cpr::Pair("has_attachment", "false"));
        datas.push_back(cpr::Pair("message_id", myNumber));
        datas.push_back(cpr::Pair("offline_threading_id", myNumber));
        datas.push_back(cpr::Pair("signature_id", "37a6884b"));
        datas.push_back(cpr::Pair("source", "source:chat:web"));
        datas.push_back(cpr::Pair("timestamp", "1484847968757"));
        datas.push_back(cpr::Pair("ui_push_phase", "V3"));
        datas.push_back(cpr::Pair("message_batch[0][thread_fbid]", std::to_string(sendTo)));
    if (!hasAttachment) {
    } else{
        datas.push_back(cpr::Pair("sharable_attachment[share_type]", "11"));
        datas.push_back(cpr::Pair("sharable_attachment[share_params][0]", "1225650124162855"));
    }

    return send(datas);
}

std::string Facechat::sendAttachement(std::string message, std::string filePath, UniversalID sendTo, bool isGroup) {
    std::vector<cpr::Pair> payloadsPairs;

    json j = uploadFile(filePath);

    std::string key;
    for (json::iterator it = j.begin(); it != j.end(); it++) {
        if (it.key().size() > 4 && it.key().substr(it.key().size() - 3) == "_id") {
            key = it.key();
            break;
        }
    }

    payloadsPairs.push_back(cpr::Pair("message_batch[0][has_attachment]", true));
    payloadsPairs.push_back(cpr::Pair("message_batch[0][" + key + "s][0]", std::to_string(j[key].get < long
    long
    int > ())));

    return sendMessage(message, sendTo, isGroup, payloadsPairs);
}

std::string Facechat::sendUrl(std::string message, std::string url, UniversalID sendTo, bool isGroup) {
    std::vector<cpr::Pair> payloadsPairs;
    payloadsPairs.push_back(cpr::Pair("message_batch[0][has_attachment]", true));
    payloadsPairs.push_back(cpr::Pair("message_batch[0][shareable_attachment][share_type]", 100));

    json j = json::parse(getFacechatURL(url));
    std::cout << j.dump(4) << std::endl;
    std::string base = "message_batch[0][shareable_attachment][share_params]";

    std::function<void(json &, std::string, std::vector<cpr::Pair> &)> eval;
    eval = [&eval](json &j, std::string base, std::vector<cpr::Pair> &pairs) {
        if (j.is_array()) {
            int x = 0;
            for (json element : j)
                eval(element, base + "[" + std::to_string(x++) + "]", pairs);
        } else if (j.is_object()) {
            for (json::iterator it = j.begin(); it != j.end(); it++)
                eval(it.value(), base + "[" + it.key() + "]", pairs);
        } else if (j.is_string())
            pairs.push_back(cpr::Pair(base, (std::string) j.get<std::string>()));
        else if (j.is_boolean())
            pairs.push_back(cpr::Pair(base, bool(j.get<bool>() ? true : false)));
    };

    eval(j, base, payloadsPairs);

    return sendMessage(message, sendTo, isGroup, payloadsPairs);
}

std::string Facechat::sendSticker(std::string stickerID, UniversalID sendTo, bool isGroup) {
    std::vector<cpr::Pair> payloadsPairs;
    payloadsPairs.push_back(cpr::Pair("message_batch[0][has_attachment]", true));
    payloadsPairs.push_back(cpr::Pair("message_batch[0][sticker_id]", stickerID));
    return sendMessage("", sendTo, isGroup, payloadsPairs);
}

std::string Facechat::createGroup(std::string message, std::vector<UserID> otherUsers) {
    std::vector<cpr::Pair> payloadsPairs;

    if (otherUsers.size() < 2)
        return "";

    for (int x = 1; x < otherUsers.size(); x++)
        payloadsPairs.push_back(cpr::Pair("message_batch[0][specific_to_list][" + std::to_string(x + 1) + "]",
                                          "fbid:" + std::to_string(otherUsers[x])));

    return sendMessage(message, otherUsers[0], false, payloadsPairs);
}

std::string Facechat::getFacechatURL(std::string url) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    payloadsPairs.push_back(cpr::Pair("image_height", 960));
    payloadsPairs.push_back(cpr::Pair("image_width", 960));
    payloadsPairs.push_back(cpr::Pair("uri", url));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {"https://www.facebook.com/message_share_attachment/fromURI/"});
    cpr::Response r = mPostSession.Post();
    helper.logJson(r, "getFacebookURL");
    return responseToJson(r)["payload"]["share_data"]["share_params"].dump();
}

json Facechat::uploadFile(std::string filePath) {
    std::vector<cpr::Part> parts;
    std::vector<cpr::Pair> payloads;
    defaultPayload(payloads);

    for (cpr::Pair &pair : payloads)
        parts.push_back({pair.key, pair.value});

    parts.push_back(cpr::Part {"upload_1024", cpr::File(filePath)});

    mPostSession.SetMultipart(cpr::Multipart {range_to_initializer_list(parts.begin(), parts.end())});
    mPostSession.SetUrl("https://upload.facebook.com/ajax/mercury/upload.php");
    cpr::Response r = mPostSession.Post();
    helper.logJson(r, "uploadFile");
    return responseToJson(r)["payload"]["metadata"][0];
}

void Facechat::deleteMessage(std::string messageID) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    payloadsPairs.push_back(cpr::Pair("client", "mercury"));
    payloadsPairs.push_back(cpr::Pair("message_ids[0]", messageID));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {delete_message_url});
    mPostSession.Post();
}

void Facechat::markAsRead(UniversalID threadID) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {set_read_status_url + "?ids[" + std::to_string(threadID) + "]=true"});
    mPostSession.Post();
}

void Facechat::setTypingStatus(UniversalID threadID, bool typing, bool isGroup) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    payloadsPairs.push_back(cpr::Pair("source", "mercury-chat"));
    payloadsPairs.push_back(cpr::Pair("thread", std::to_string(threadID)));
    payloadsPairs.push_back(cpr::Pair("typ", (int) typing));
    payloadsPairs.push_back(cpr::Pair("to", (!isGroup ? std::to_string(threadID) : "")));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {set_typing_status_url});
    mPostSession.Post();
}

void Facechat::setGroupTitle(UniversalID id, std::string title) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);
    helper.setGroupTitle(payloadsPairs, id, title, mUserID);
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {send_message_url});
    mPostSession.Post();
}

void Facechat::addUserToGroup(UserID userID, ThreadID group) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);
    helper.addUserToGroup(payloadsPairs, userID, group, mUserID);
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {send_message_url});
    mPostSession.Post();
}

void Facechat::removeUserFromGroup(UserID userID, ThreadID group) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    payloadsPairs.push_back(cpr::Pair("uid", std::to_string(userID)));
    payloadsPairs.push_back(cpr::Pair("tid", std::to_string(group)));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {remove_user_from_group_url});
    mPostSession.Post();
}

std::vector<UserID> Facechat::getFriendList(UserID id) {
    std::vector<UserID> friends;
    try {
        std::vector<cpr::Pair> payloadsPairs;
        defaultPayload(payloadsPairs);
        payloadsPairs.push_back(cpr::Pair("dpr", "1"));
        payloadsPairs.push_back(cpr::Pair("dyn",
                                          "7AmajEzUGByA5Q9UoGyk4C5oWq2WiWF298yfirWo8popyUWdwIhE98nwgUy22EaUgxebkwy8xa5WjzEgDKuEjKewExaFQ12VVojxCVEiHWCDxi5-uifz8lUhK1iyECiumFoK48hxB2Uf8oC_UjDQ6EvGi64iiamezES68G5F99Ft5xq498lBVpEjw"));
        payloadsPairs.push_back(cpr::Pair("__af", "i0"));
        payloadsPairs.push_back(cpr::Pair("__be", "-1"));
        payloadsPairs.push_back(cpr::Pair("__pc", "PHASED:DEFAULT"));
        mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
        mPostSession.SetUrl(cpr::Url {get_friends_list_part_1 + std::to_string(id)});
        cpr::Response r = mPostSession.Get();
//    helper.logJson(r, "getFriendsList");


//    friends.reserve(2000);

        int pos = r.text.find("AllFriendsAppCollectionPagelet");
        if (pos == std::string::npos) {
            std::cout << "pos1 not found\n";
            pos = r.text.find("FriendsAppCollectionPagelet");
        }
        if (pos == std::string::npos) {
            std::cout << "pos2 not found\n";
            return friends;
        }

        r.text.erase(0, pos);
        r.text.erase(0, r.text.find("\"token\":\"") + 9);
        std::string token = r.text.substr(0, r.text.find("\""));

        pos = r.text.find("&quot;eng_tid&quot;:&quot;");
        r.text = r.text.substr(pos, r.text.rfind("&quot;eng_tid&quot;:&quot;") + 55 - pos);

        pos = 0;
        while ((pos = r.text.find("&quot;eng_tid&quot;:&quot;", pos) + 26) != std::string::npos + 26)
            friends.push_back(std::stoll(r.text.substr(pos, r.text.find("&quot;", pos) - pos)));

        json data;
        data["collection_token"] = token;
        data["tab_key"] = "friends";
        data["sk"] = "friends";
        data["profile_id"] = id;
        data["overview"] = false;
        data["ftid"].clear();
        data["order"].clear();
        data["importer_state"].clear();

        payloadsPairs.push_back(cpr::Pair("", ""));

        for (int i = 0; i < 50; i++) {
            try {

                data["lst"] = mUserID + ":" + std::to_string(id) + ":1485798526";
                data["cursor"] = encodeBase64("0:not_structured:" + std::to_string(friends.back()));

                payloadsPairs.pop_back();

                payloadsPairs.push_back(cpr::Pair("data", data.dump()));
                mPostSession.SetUrl(cpr::Url {get_friends_list_part_2});
                mPostSession.SetPayload(
                        cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
                r = mPostSession.Post();
                token = std::to_string(id) + ":2356318349:2";

                json j = responseToJson(r)["jsmods"]["require"];

//        std::cout << "responseToJson: " << j.dump() << std::endl;
                for (auto element : j) {
                    if (element[0] == "AddFriendButton") {
                        auto newfriend = element[3][1];
//                std::cout <<"el1: "<<element[3][0]<<std::endl;
                        //                std::cout << "newfriend: " << newfriend << std::endl;
                        friends.push_back(newfriend);
                    }
                }


                data["collection_token"] = token;

            } catch (std::exception &e) {
                std::cout << "Exception in  getFriendList loop " << e.what() << std::endl;
            }
        }
        std::cout << "Found friends: " << friends.size() << std::endl;
        sort(friends.begin(), friends.end());
        friends.erase(unique(friends.begin(), friends.end()), friends.end());
        std::random_shuffle ( friends.begin(), friends.end());
        std::cout << "Uniqued friends: " << friends.size() << std::endl;
    } catch (std::exception &e) {
        std::cout << "Exception in getFriendList " << e.what() << std::endl;
    }
    return friends;
}

std::vector<std::pair<UserID, time_t>> Facechat::getOnlineFriend(bool includeMobile) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    payloadsPairs.push_back(cpr::Pair("user", mUserID));
    payloadsPairs.push_back(cpr::Pair("fetch_mobile", (includeMobile ? true : false)));
    payloadsPairs.push_back(cpr::Pair("get_now_available_list", true));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {get_online_friends_url});
    cpr::Response r = mPostSession.Post();
    std::cout << "getOnlineFriend: " << r.text << std::endl;
//    for(auto& c : r.cookies.map_)
//        std::cout << c.first << ": " << c.second << std::endl;
    json j = responseToJson(r)["payload"]["buddy_list"];

    std::vector<std::pair<UserID, time_t>> onlines;

    for (json::iterator it = j["nowAvailableList"].begin(); it != j["nowAvailableList"].end(); it++) {
        UserID id = std::stoll(it.key());
        json jsonTimestamp = j["last_active_times"][std::to_string(id)];
        time_t timestamp = (jsonTimestamp.is_number() ? jsonTimestamp.get<time_t>() : 0);
        onlines.push_back(std::pair<UserID, time_t>(id, timestamp));
    }

    if (j["mobile_friends"].is_array()) {
        for (UserID id : j["mobile_friends"]) {
            json jsonTimestamp = j["last_active_times"][std::to_string(id)];
            time_t timestamp = (jsonTimestamp.is_number() ? jsonTimestamp.get<time_t>() : 0);
            onlines.push_back(std::pair<UserID, time_t>(id, timestamp));
        }
    }

    return onlines;
}

Facechat::UserInfo Facechat::getUserInfo(UserID id) {
    std::vector<cpr::Pair> payloadsPairs;
    UserInfo info;
    try {
        defaultPayload(payloadsPairs);


        payloadsPairs.push_back(cpr::Pair("ids[0]", std::to_string(id)));

        mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
        mPostSession.SetUrl(cpr::Url {get_user_info});
        cpr::Response r = mPostSession.Post();

        std::string infoStr = "https://www.facebook.com/profile.php?id=";
        infoStr.append(std::to_string(id));
        payloadsPairs.pop_back();
        mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
        mPostSession.SetUrl(cpr::Url {infoStr});
        cpr::Response re2 = mPostSession.Get();

        int friendsCount = 0;
        try {
            size_t pos = re2.text.find("wspólnych znajomych");
            if (pos != std::string::npos) {
                auto almostInt = re2.text.substr(pos - 4, 4);
                friendsCount = extractIntFromString(almostInt);
            }
        } catch (std::exception &e) {
            std::cout << "Excepttion in getUserInfo 0  " << e.what() << std::endl;
        }

//    helper.logJson(r, "getUserInfo");
        json j;
        try {
            j = responseToJson(r)["payload"]["profiles"][std::to_string(id)];
        } catch (std::exception &e) {
            std::cout << "Exception in  getUserInfo 1" << e.what() << std::endl;
            return info;
        }

        info.isValid=false;
        try {
            info.completeName = j["name"].get<std::string>();
            info.firstName = j["firstName"].get<std::string>();
            info.gender = j["gender"];
            info.isFriend = j["is_friend"];
            info.id = std::stoll(j["id"].get<std::string>());
            info.profilePicture = j["thumbSrc"].get<std::string>();
            info.profileUrl = j["uri"].get<std::string>();
            info.vanity = j["vanity"].get<std::string>();
            info.friendsCount = friendsCount;
            info.isValid=true;
        } catch (std::exception &e) {
            std::cout << "Exception in  getUserInfo 2" << e.what() << std::endl;
            return info;
        }
    } catch (std::exception &e) {
        std::cout << "Excepttion: getUserInfo 4" << e.what() << std::endl;
    }
    return info;
}

std::vector<Facechat::UserSearchReturn> Facechat::findUser(std::string name) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    payloadsPairs.push_back(cpr::Pair("viewer", mUserID));
    payloadsPairs.push_back(cpr::Pair("rsp", "search"));
    payloadsPairs.push_back(cpr::Pair("context", "search"));
    payloadsPairs.push_back(cpr::Pair("path", "/home.php"));
    payloadsPairs.push_back(cpr::Pair("value", name));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {find_user_url});
    cpr::Response r = mPostSession.Post();
    helper.logJson(r, "findUser");
    json j = responseToJson(r)["payload"]["entries"];

    std::vector<UserSearchReturn> users;
    for (json element : j) {
        if (element["type"] != "user")
            continue;

        UserSearchReturn user;
        user.id = element["uid"];
        user.index = element["index_rank"];
        user.score = element["score"];
        user.name = element["names"][0].get<std::string>();
        user.profilePicture = element["photo"].get<std::string>();
        user.profileUrl = element["path"].get<std::string>();
        users.push_back(user);
    }

    return users;
}

Facechat::Thread Facechat::getThread(UniversalID id) {
    int offset = 0;
    int end = 0;
    while (!end) {
        end = 1;
        for (Thread &thread : listThread(offset, 30)) {
            end = 0;
            if (thread.threadID == id)
                return thread;

            offset++;
        }
    }
}

std::vector<Facechat::Thread> Facechat::listThread(int offset, int limit) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);
    payloadsPairs.push_back(cpr::Pair("inbox[offset]", offset));
    payloadsPairs.push_back(cpr::Pair("inbox[limit]", limit));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {list_thread_url});
    cpr::Response r = mPostSession.Post();

    std::vector<Facechat::Thread> threads;

    if (r.status_code) {
        json j = responseToJson(r)["payload"]["threads"];
        threads.reserve(j.size());

        for (json::iterator it = j.begin(); it != j.end(); ++it)
            threads.push_back(parseThread(*it));
    }

    return threads;
}

std::vector<Facechat::Thread> Facechat::findThread(std::string name, int offset, int limit) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);
    payloadsPairs.push_back(cpr::Pair("query", name));
    payloadsPairs.push_back(cpr::Pair("offset", (int) offset));
    payloadsPairs.push_back(cpr::Pair("limit", (int) limit));
    payloadsPairs.push_back(cpr::Pair("index", "fbid"));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {find_thread_url});
    cpr::Response r = mPostSession.Post();

    std::vector<Facechat::Thread> threads;

    if (r.status_code) {
        json j = responseToJson(r)["payload"]["mercury_payload"]["threads"];
        threads.reserve(j.size());

        for (json::iterator it = j.begin(); it != j.end(); ++it)
            threads.push_back(parseThread(*it));
    }

    return threads;
}


std::vector<Facechat::Message>
Facechat::readThread(UniversalID id, int offset, int limit, time_t timestamp, bool isGroup) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    const std::string key = isGroup ? "thread_fbids" : "user_ids";

    payloadsPairs.push_back(cpr::Pair("messages[" + key + "][" + std::to_string(id) + "][offset]", (int) offset));
    payloadsPairs.push_back(cpr::Pair("messages[" + key + "][" + std::to_string(id) + "][limit]", (int) limit));
    payloadsPairs.push_back(
            cpr::Pair("messages[" + key + "][" + std::to_string(id) + "][timestamp]", std::to_string(timestamp)));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});

    mPostSession.SetUrl(cpr::Url {thread_info_url});

    cpr::Response r = mPostSession.Post();

    std::cout << "readThread: " << responseToJson(r).dump(4) << std::endl;;

    std::vector<Facechat::Message> messages;

    if (r.status_code) {
        json j = responseToJson(r);
        j = j["payload"]["actions"];

        if (j.is_array()) {
            for (json action : j) {
                if (action["action_type"].is_null())
                    break;
                try {
                    if (action["action_type"] == "ma-type:user-generated-message")
                        messages.push_back(parseMessage(action));
                }
                catch (...) {}
            }
        }
    }

    return messages;
}

void Facechat::deleteThread(UniversalID id) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);
    payloadsPairs.push_back(cpr::Pair("client", "mercury"));
    payloadsPairs.push_back(cpr::Pair("ids[0]", std::to_string(id)));

    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {delete_thread_url});
    cpr::Response r = mPostSession.Post();
    helper.logJson(r, "deleteThread");

}

Facechat::Message Facechat::parseMessage(json &j) {
    Facechat::Message message;

    message.body = (j["body"].is_string() ? j["body"].get<std::string>() : "");
    message.senderName = (j["sender_name"].is_string() ? j["sender_name"].get<std::string>() : "");
    message.messageID = (j["mid"].is_string() ? j["mid"].get<std::string>() : "");
    message.messageID = (j["message_id"].is_string() ? j["message_id"].get<std::string>() : message.messageID);
    message.to = (j["other_user_fbid"].is_number() ? j["other_user_fbid"].get < long long
    int > () : 0);

    message.timestamp = (j["timestamp"].is_number() ? j["timestamp"].get<time_t>() : 0);

    message.from = (j["sender_fbid"].is_number() ? j["sender_fbid"].get < long long
    int > () : 0);
    if (!message.from) {
        std::string id = (j["author"].is_string() ? j["author"].get<std::string>() : "");
        if (!id.empty() && (id.substr(0, 5) == "fbid:"))
            message.from = std::stoll(id.erase(0, 5));

        assert(message.from);
    }

    std::string threadID = (j["thread_fbid"].is_string() ? j["thread_fbid"].get<std::string>() : "");
    if (!threadID.empty()) {
        if (threadID[0] == ':')
            threadID.erase(0, 1);

        message.conversationID = std::stoll(threadID);
    }

    if (j["group_thread_info"].is_object()) {
        message.isGroup = true;
        message.group.groupParticipantCount = j["group_thread_info"]["participant_total_count"];

        for (const UserID &id : j["group_thread_info"]["participant_ids"])
            message.group.groupParticipantIDs.push_back(id);

        for (const std::string &name : j["group_thread_info"]["participant_names"])
            message.group.groupParticipantNames.push_back(name);
    }

    if (j["has_attachment"]) {
        message.asAttachment = true;

        message.attachment.type = Attachment::stringToAttachmentType(j["attachments"][0]["attach_type"]);

        if (message.attachment.type == Attachment::STICKER) {
            message.attachment.stickerID = j["attachments"][0]["metadata"]["stickerID"];

            message.attachment.url = j["attachments"][0]["url"].get<std::string>();
            message.attachment.previewUrl = message.attachment.url;
        } else if (message.attachment.type == Attachment::PHOTO) {
            message.attachment.url = j["attachments"][0]["hires_url"].get<std::string>();
            message.attachment.previewUrl = j["attachments"][0]["preview_url"].get<std::string>();
        } else if (message.attachment.type == Attachment::VIDEO) {
            message.attachment.url = j["attachments"][0]["url"].get<std::string>();
            message.attachment.previewUrl = j["attachments"][0]["preview_url"].get<std::string>();
        } else
            message.asAttachment = false;
    }

    return message;
}

Facechat::Message Facechat::parseUpdatMessage(json &j) {
    Facechat::Message message;

    auto mm = j["messageMetadata"];
    std::cout << "parseUpdatMessage: " << mm.dump(3) << std::endl;

    message.body = (j["body"].is_string() ? j["body"].get<std::string>() : "");
    message.messageID = mm["messageId"].get<std::string>();
    message.to = std::stoll(mm["threadKey"]["otherUserFbId"].get<std::string>());
    message.timestamp = std::stoll(mm["timestamp"].get<std::string>());
    message.from = std::stoll(mm["actorFbId"].get<std::string>());
    message.conversationID = std::stoll(mm["threadKey"]["otherUserFbId"].get<std::string>());
    return message;
}

Facechat::Thread Facechat::parseThread(json &j) {
    Thread thread;
    thread.messageCount = j["message_count"];
    thread.name = j["name"].get<std::string>();
    thread.threadID = std::stoll(j["thread_fbid"].get<std::string>());

    for (std::string id : j["participants"])
        thread.participants.push_back(std::stoll(id.substr(std::string("fbid:").size())));


    for (json id : j["former_participants"])
        thread.pastParticipant.push_back(std::stoll((id["id"]).get<std::string>().substr(std::string("fbid:").size())));

    return thread;
}

void Facechat::searchForUserPosts(bool extended) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    std::string url = "https://www.facebook.com/slawomirmentzen/?fref=nf&pnref=story";
    mPostSession.SetUrl(cpr::Url {url});
    cpr::Response r = mPostSession.Get();
    helper.logJson(r, "graphApi", false);

    std::vector<std::string> postsPositions = helper.parseResponseForUserPosts(r);
    /* Print unique resoults*/
    for (auto s: postsPositions) {
        std::cout << s << std::endl;
    }
    /*read ownerid*/
    //ownerid:"1002104843184052
    auto poss = r.text.find("ownerid");
    if (poss == std::string::npos) {
        std::cout << "Couldn't find ownerid" << std::endl;
        return;
    }
    std::string owner_id = r.text.substr(poss + 9, 16);
    std::cout << "owner_id " << owner_id << std::endl;
    /*try to load more posts*/
    helper.loadMorePosts(payloadsPairs, owner_id);

//    std::vector<cpr::Pair> payloadsPairsNew;
//    defaultPayload(payloadsPairsNew);
//    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairsNew.begin(), payloadsPairsNew.end())});
//    std::string  loadMorePostsUrl="https://www.facebook.com/pages_reaction_units/more/";
    //https://www.facebook.com/pages_reaction_units/more/?
    // page_id=1002104843184052&
    // cursor={"timeline_cursor":"timeline_unit:1:00000000001485276972:04611686018427387904:09223372036854775807:04611686018427387904","timeline_section_cursor":{},"has_next_page":true}&
    // surface=www_pages_home&
    // unit_count=8&
    // dpr=1.5&
    // __user=100000448934799& .
    // __a=1&  .
    // __dyn=aKhoFeyfyGmaomgDBUOWEyAzm5ubhEK5EKiWFaayedirWqF1zBxvCRAyqyEszQHUGaUF7zFGxK5FFHxuqE88HyZ7yUJi28yuaxuAUWVpfLKtojKeyVohWxaFTxG2jDh8LBxDVF8SLQhDBzA5Kuifz8gAVCcy46o-ElByFGADh8zyogyVoWbCAwBxqmuaC_QfHQJ129z8y9Gi4rld4yByWBhRye5KlG4p8CHBRS5EgAxmnAhEGrw&
    // __af=i0&
    // __req=26&   .
    // __be=-1&
    // __pc=PHASED:DEFAULT&  .
    // __rev=2796038   .
//    mPostSession.SetUrl(cpr::Url {loadMorePostsUrl});
//    mPostSession.SetUrl(cpr::Url {"https://www.facebook.com/pages_reaction_units/more/?page_id=1002104843184052&cursor={\"timeline_cursor\":\"timeline_unit:1:00000000001485276972:04611686018427387904:09223372036854775807:04611686018427387904\",\"timeline_section_cursor\":{},\"has_next_page\":true}&surface=www_pages_home&unit_count=8&dpr=1.5&__user=100000448934799&__a=1&__dyn=aKhoFeyfyGmaomgDBUOWEyAzm5ubhEK5EKiWFaayedirWqF1zBxvCRAyqyEszQHUGaUF7zFGxK5FFHxuqE88HyZ7yUJi28yuaxuAUWVpfLKtojKeyVohWxaFTxG2jDh8LBxDVF8SLQhDBzA5Kuifz8gAVCcy46o-ElByFGADh8zyogyVoWbCAwBxqmuaC_QfHQJ129z8y9Gi4rld4yByWBhRye5KlG4p8CHBRS5EgAxmnAhEGrw&__af=i0&__req=26&__be=-1&__pc=PHASED:DEFAULT&__rev=2796038"});
//    cpr::Response re = mPostSession.Get();
//    helper.logJson(re, "loadMorePost", true);
//    std::vector<std::string> postsPositionsExtended = helper.parseResponseForUserPosts(re);
//    for(auto s: postsPositionsExtended){
//        std::cout<<s<<std::endl;
//    }


    if (postsPositions.size() < 1)
        return;
}

void Facechat::updateStatus() {
    std::string composer_id = "rc.u_0_v"; //
    std::vector<cpr::Pair> payloadsPairsNew;
    defaultPayload(payloadsPairsNew);
    helper.reactComposer(payloadsPairsNew, composer_id);
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairsNew.begin(), payloadsPairsNew.end())});
    std::string react_composerUrl = "https://www.facebook.com/react_composer/scraper/";
//    std::string  react_composerUrl2="https://www.facebook.com/react_composer/scraper/?composer_id=rc.js_5he&target_id=100014792015409&scrape_url=https://www.facebook.com/slawomirmentzen/videos/1225650124162855/&entry_point=timeline&source_attachment=STATUS&source_logging_name=link_pasted&av=100014792015409&dpr=1.5";
    mPostSession.SetUrl(cpr::Url {react_composerUrl});
    cpr::Response react_composerRes = mPostSession.Post();
    helper.logJson(react_composerRes, "react_composer", true);

    std::vector<cpr::Pair> payloadsPairsNew2;
    std::string updateStatusUrl = "https://www.facebook.com/ajax/updatestatus.php?av=100000448934799&dpr=1.5";
    mPostSession.SetUrl(cpr::Url {updateStatusUrl});
    defaultPayload(payloadsPairsNew);
    helper.updateStatus(payloadsPairsNew2, composer_id);
    mPostSession.SetPayload(
            cpr::Payload {range_to_initializer_list(payloadsPairsNew2.begin(), payloadsPairsNew2.end())});
    cpr::Response updateStatusRes = mPostSession.Post();
    helper.logJson(updateStatusRes, "updateStatus", true);
    //https://www.facebook.com/ajax/updatestatus.php?av=100000448934799&dpr=1.5
}


void Facechat::defaultPayload(std::vector<cpr::Pair> &payloadsPairs) {
    if (mDefaultPayloads.empty()) {
//        mDefaultPayloads.push_back(cpr::Pair("client", "mercury"));
        mDefaultPayloads.push_back(cpr::Pair("__user", mUserID));
        mDefaultPayloads.push_back(cpr::Pair("__a", 1));
        mDefaultPayloads.push_back(cpr::Pair("__req", "u"));// bylo 3
        mDefaultPayloads.push_back(cpr::Pair("fb_dtsg", mDTSG));

        std::string ttstamp = "";
        for (const char c : mDTSG)
            ttstamp += std::to_string((int) c);
        ttstamp += '2';

        mDefaultPayloads.push_back(cpr::Pair("ttstamp", ttstamp));
        mDefaultPayloads.push_back(cpr::Pair("__rev", mRevision));
    }
    payloadsPairs.insert(payloadsPairs.begin(), mDefaultPayloads.begin(), mDefaultPayloads.end());
}

bool Facechat::searchFriendInvitationList() {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    std::string url = "https://www.facebook.com/?sk=ff";
    mPostSession.SetUrl(cpr::Url {url});
    cpr::Response r = mPostSession.Get();
    helper.logJson(r, "searchFriendInvitationList");
    std::string sub = "Osoby, kt"; //Only for Polish facebook - 'People who can you know'
    size_t posAcceptEnd = r.text.find(sub);
    std::vector<std::string> gotInvitations;
    std::vector<std::string> sendInvitationsTo;
    size_t pos1 = 0;
    // size_t pos2 = 0;
    size_t pos3 = 0;
    sub = "user.php?id=";
    while ((pos1 = r.text.find(sub, pos3 + 1)) != std::string::npos) {
        try {
            // pos2=r.text.find("&", pos1 + 1);
            pos3 = r.text.find_first_of("\"&", pos1 + 1);
            // if(pos2<pos3){
//                if(pos1<posAcceptEnd)  {
//                    gotInvitations.push_back(r.text.substr(pos1,pos2-pos1));
//                    std::cout<<"ID osoby do zaproszenia"<<r.text.substr(pos1,pos2-pos1)<<std::endl;
//                }
//                else {
//                    toSendInvitations.push_back(r.text.substr(pos1, pos2 - pos1));
//                }
//           // }
            if (r.text[(r.text.find("a>", pos3)) + 2] == '<') {
                if (pos1 < posAcceptEnd) {
                    std::string FoundID = r.text.substr(pos1 + sub.length(), pos3 - pos1 - sub.length());
                    gotInvitations.push_back(FoundID);
                    std::cout << "ID osoby do zaproszenia" << FoundID << std::endl;
                    acceptInvitation(FoundID);
                } else {
                    std::string FoundID = r.text.substr(pos1 + sub.length(), pos3 - pos1 - sub.length());
                    sendInvitationsTo.push_back(FoundID);
                    std::cout << "ID osoby ktora mozesz znac" << FoundID << std::endl;
                    sendInvitation(FoundID);
                }
            }
        } catch (std::out_of_range &exception) {
            std::cout << "Exception in searchFriendInvitationList\n";
        }
    }
    std::cout << "Counts of gotInvitations: " << gotInvitations.size() << std::endl;
    std::cout << "Counts of sentInvitaions " << sendInvitationsTo.size() << std::endl;

    return 0;


}


bool Facechat::acceptInvitation(std::string Id) {
    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    payloadsPairs.push_back(cpr::Pair("id", Id));
    payloadsPairs.push_back(cpr::Pair("action", "confirm"));
    payloadsPairs.push_back(cpr::Pair("viewer_id", mUserID));
    payloadsPairs.push_back(cpr::Pair("floc:", "friend_center_requests"));
    payloadsPairs.push_back(cpr::Pair("ref:", "reqs.php"));

    std::string url = "https://www.facebook.com/requests/friends/ajax/?dpr=1";
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {url});
    cpr::Response r = mPostSession.Post();
//    helper.logJson(r, "sendInvitation");
    return 0;
}

bool Facechat::sendInvitation(std::string Id) {
    try {

        std::vector<cpr::Pair> payloadsPairs;
        defaultPayload(payloadsPairs);

        payloadsPairs.push_back(cpr::Pair("to_friend", Id));
        payloadsPairs.push_back(cpr::Pair("action", "add_friend"));
        payloadsPairs.push_back(cpr::Pair("how_found", "requests_page_pymk"));
        payloadsPairs.push_back(cpr::Pair("ref_param", "none"));
        payloadsPairs.push_back(cpr::Pair("logging_location", "friends_center"));

        std::string url = "https://www.facebook.com/ajax/add_friend/action.php?dpr=1";
        mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
        mPostSession.SetUrl(cpr::Url {url});
        cpr::Response r = mPostSession.Post();
//        helper.logJson(r, "acceptINvitation");
    } catch (std::exception &e) {
        std::cout << "Exception in sendInvitation " << e.what() << std::endl;
    }
    return 0;
}

bool Facechat::searchSendInvitationsList() {
    std::vector<std::string> recallInvitations;
    std::vector<std::string> sendInvitationsTo;
    while (recallInvitations.size() != 0) {
        recallInvitations.clear();
        sendInvitationsTo.clear();
        std::vector<cpr::Pair> payloadsPairs;
        // payloadsPairs.push_back(cpr::Pair("outgoing","1"));
        defaultPayload(payloadsPairs);
        // payloadsPairs.push_back(cpr::Pair("fcref","jwl"));


        mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
        std::string url = "https://www.facebook.com/friends/requests/?fcref=jwl&outgoing=1";
        mPostSession.SetUrl(cpr::Url {url});
        cpr::Response r = mPostSession.Get();
        helper.logJson(r, "searchSendInvitationList");

        std::string sub = "Osoby, kt"; //Only for Polish facebook - 'People who can you know'
        size_t posAcceptEnd = r.text.find(sub);

        size_t pos1 = 0;
        // size_t pos2 = 0;
        size_t pos3 = 0;
        sub = "user.php?id=";
        while ((pos1 = r.text.find(sub, pos3 + 1)) != std::string::npos) {
            try {
                pos3 = r.text.find_first_of("\"&\\", pos1 + 1);
                if (r.text[(r.text.find("a>", pos3)) + 2] == '<') {
                    if (pos1 < posAcceptEnd) {
                        std::string FoundID = r.text.substr(pos1 + sub.length(), pos3 - pos1 - sub.length());
                        recallInvitations.push_back(FoundID);
                        std::cout << "ID osoby cofniacia zaproszenia" << FoundID << std::endl;
                        recallSendInvitations(FoundID);
                    }
//                else {
//                    std::string FoundID = r.text.substr(pos1+sub.length(),pos3-pos1-sub.length());
//                    sendInvitationsTo.push_back(FoundID);
//                    std::cout<<"ID osoby ktora mozesz znac"<<FoundID<<std::endl;
//                    sendInvitation(FoundID);
//                }
                }
            } catch (std::out_of_range &exception) {
                std::cout << "Exception in searchSendInvitationList\n";
            }
        }

        std::cout << "Counts of recallInvitations: " << recallInvitations.size() << std::endl;
        std::cout << "Counts of sentInvitaions " << sendInvitationsTo.size() << std::endl;
    }
    return 0;


}

bool Facechat::recallSendInvitations(std::string Id) {

    std::vector<cpr::Pair> payloadsPairs;
    defaultPayload(payloadsPairs);

    payloadsPairs.push_back(cpr::Pair("friend", Id));
    payloadsPairs.push_back(cpr::Pair("cancel_ref", "outgoing_requests"));
    payloadsPairs.push_back(cpr::Pair("confirmed", "1"));

    std::string url = "https://www.facebook.com/ajax/friends/requests/cancel.php?dpr=1";
    mPostSession.SetPayload(cpr::Payload {range_to_initializer_list(payloadsPairs.begin(), payloadsPairs.end())});
    mPostSession.SetUrl(cpr::Url {url});
    cpr::Response r = mPostSession.Post();
    helper.logJson(r, "recallINvitation");

    return 0;
}


