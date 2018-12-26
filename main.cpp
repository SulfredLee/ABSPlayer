#include <iostream>
#include <string>
#include "mplayerManager.h"
#include "PlayerMsg_Common.h"

#include <unistd.h>

int main(int argc, char* argv[])
{
    mplayerManager manager;
    manager.InitComponent();
    cmdReceiver* cmdHandler = static_cast<cmdReceiver*>(&manager);
    while(true)
    {
        std::cout << "Your action:" << std::endl
                  << "1: Open" << std::endl
                  << "2: Play" << std::endl
                  << "3: Pause" << std::endl
                  << "4: Stop" << std::endl;
        int action; std::string url;
        std::cin >> action;
        switch (action)
        {
            case 1:
                {
                    std::cout << "Input url" << std::endl;
                    std::cin >> url;
                    PlayerMsg_Open* msg = new PlayerMsg_Open();
                    msg->SetURL(url);
                    PlayerMsg_Base* msgBase = static_cast<PlayerMsg_Base*>(msg);
                    cmdHandler->UpdateCMDReceiver(msgBase);
                    break;
                }
            case 2:
                {
                    PlayerMsg_Play* msg = new PlayerMsg_Play();
                    PlayerMsg_Base* msgBase = static_cast<PlayerMsg_Base*>(msg);
                    cmdHandler->UpdateCMDReceiver(msgBase);
                    break;
                }
            case 3:
                {
                    PlayerMsg_Pause* msg = new PlayerMsg_Pause();
                    PlayerMsg_Base* msgBase = static_cast<PlayerMsg_Base*>(msg);
                    cmdHandler->UpdateCMDReceiver(msgBase);
                    break;
                }
            case 4:
                {
                    PlayerMsg_Stop* msg = new PlayerMsg_Stop();
                    PlayerMsg_Base* msgBase = static_cast<PlayerMsg_Base*>(msg);
                    cmdHandler->UpdateCMDReceiver(msgBase);
                    break;
                }
            default:
                break;
        }
    }
    return 0;
}
