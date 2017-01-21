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
    f.login(email, password);
    UserID userID{100008278386721};
    UniversalID universalID{userID};
    //auto info = f.getUserInfo(userID);
    std::cout<<"End getUserInfo\n";
    std::this_thread::sleep_for(std::chrono::seconds(4));




    f.sendMessage("Czesc :)1", userID, false);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    f.sendMessage("Czesc :)2", userID, false);

    while (1) {}
}

