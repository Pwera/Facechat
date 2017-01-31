#include <cpr/cpr.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>
//#include "rmem/rmem_entry.h"
//#include "rmem/rmem.h"
#include "Facechat.h"

int main() {
    auto email = "";
    auto password = "";
    Facechat f;
    if (f.login(email, password) == 0) {
        std::cout << "-----------------------LOGIN FAILED-----------------------\n";
        return -1;
    }

    UserID piotrWera{100000448934799};
    UserID userID{100014792015409};

//    std::cout<<"End getUserInfo\n";
//    std::this_thread::sleep_for(std::chrono::seconds(4));
//    std::vector<UserID> friends= f.getFriendList(userID);
//    for (auto& fr : friends){
//        Facechat::UserInfo us = f.getUserInfo(fr);
//        std::cout<<us.completeName<<std::endl;
//    }

//    std::cout<<"Friends count : " << friends.size()<<std::endl;
//    friends.clear();
//    friends = f.getFriendList(userID);
//    std::cout<<"Friends count : " << friends.size()<<std::endl;


//    htmlToListOfFriends();

//    std::this_thread::sleep_for(std::chrono::seconds(2));

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



//    f.deleteThread(szymonMadera);
//    v.clear();
//    v = f.readThread(szymonMadera);
//    std::cout<<"v size: "<<v.size()<<std::endl;

    auto myFriends = f.getFriendList(userID);
//        auto myFriends2 = f.getFriendList(userID);

//        std::merge(myFriends.begin(), myFriends.end(), myFriends2.begin(), myFriends2.end(), std::back_inserter(dst));
//        sort(dst.begin(), dst.end());
//        dst.erase(unique(dst.begin(), dst.end()), dst.end());
//                std::cout<<"dst size: "<<dst.size()<<std::endl;

    std::cout << "myFriends" << myFriends[20] << std::endl;
    auto myFriends2 = f.getFriendList(myFriends[0]);
    try {
        for (auto &us : myFriends2) {
            auto info = f.getUserInfo(us);
            std::cout << "us: " << info.completeName << " FR:" << info.isFriend << info.id<<std::endl;
        }
    } catch (std::exception &e) {
        std::cout << "Exception in  main" << e.what() << std::endl;
    }
    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
//        auto v = f.readThread(piotrWera);
//        std::cout<<"v size: "<<v.size()<<std::endl;
    }

}

