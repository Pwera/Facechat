#include <cpr/cpr.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include "Facechat.h"
void inviteMoreFriends(Facechat& f, UserID &userID);
int main() {
    auto email = "dupa7";
    auto password = "";
    Facechat f;
    if (f.login(email, password) == 0) {
        std::cout << "-----------------------LOGIN FAILED-----------------------\n";
        return -1;
    }

    UserID piotrWera{100000448934799};
    UserID userID{100014792015409};
    //    std::this_thread::sleep_for(std::chrono::seconds(4));



//    for(int i =0;i<30;i++){
//        std::this_thread::sleep_for(std::chrono::seconds(1));
//        std::string s = "#$% ";
//        try {
//            f.sendMessage(s.append(std::to_string(i)), szymonMadera, false);
//        }catch(std::invalid_argument& e){
//            std::cout<<"Exception, std::invalid_argument "<<e.what()<<std::endl;
//            continue;
//        }
//    }

    std::string s="Cześćhttps://www.facebook.com/slawomirmentzen/videos/1225650124162855/";
    f.sendMessage(s, piotrWera, true);

  //  inviteMoreFriends(f, userID);

    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(1000));
//        auto v = f.readThread(piotrWera);
//        std::cout<<"v size: "<<v.size()<<std::endl;
    }

}

void inviteMoreFriends(Facechat& f, UserID &userID){
    int invited = 0;
    try {
        auto myFriends = f.getFriendList(userID);
        for (auto &us : myFriends) {
            auto myFriends2 = f.getFriendList(us);
            auto ii = f.getUserInfo(us);
            if(!ii.isValid){
                continue;
            }
            std::cout << "Friends of : "<<ii.completeName <<" "<<ii.id<<" " << myFriends2.size()<<std::endl;
            for (auto &us : myFriends2) {
                auto info = f.getUserInfo(us);
                if(!info.isValid){
                    continue;
                }
                std::cout << "us: " << info.completeName << " FR:" << info.isFriend << " " << info.id << std::endl;
                if (!info.isFriend && info.friendsCount > 10) {
                    try {
                        f.sendInvitation(std::to_string(info.id));
                        std::cout << "Wyslano zaproszenie do " << std::to_string(info.id) <<  " liczba zaproszonych: "  <<invited++<< std::endl;
                    } catch (std::exception &e) {
                        std::cout << "Exception, pzy wysylaniu zaproszenie do " << std::to_string(info.id) << std::endl;
                    }
                }
            }
        }
    } catch (...) {
        std::cout << "Exception in  mainn" <<  std::endl;
    }
}
