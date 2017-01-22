#include <cpr/cpr.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include "Facechat.h"
int main() {
    auto email = "";
    auto password = "";
    Facechat f;
    if(f.login(email, password)==0){
        std::cout<<"-----------------------LOGIN FAILED-----------------------\n";
        return -1;
    }

    UserID userID{100008278386721};
    UniversalID universalID{userID};
    std::cout<<"End getUserInfo\n";
    std::this_thread::sleep_for(std::chrono::seconds(4));
//    f.logout();
//    return 0;

//    f.sendMessage("Czesc :)1", userID, false);
//    std::this_thread::sleep_for(std::chrono::seconds(2));
//    f.sendMessage("Czesc :)2", userID, false);
    try {
        f.grephApi();
    }catch(std::exception& e){
        std::cout<<"Exception: "<<e.what()<<std::endl;
    }
    while (1) {}
}

