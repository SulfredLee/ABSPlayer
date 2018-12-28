#include <iostream>
#include <string>
#include "mplayerManager.h"
#include "PlayerMsg_Common.h"

#include <unistd.h>
#include <memory>

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
                    for (int i = 0; i < 10; i++)
                    {
                        std::shared_ptr<PlayerMsg_Open> msg = std::make_shared<PlayerMsg_Open>();
                        msg->SetURL(url);
                        std::shared_ptr<PlayerMsg_Base> msgBase = std::static_pointer_cast<PlayerMsg_Base>(msg);
                        cmdHandler->UpdateCMDReceiver(msgBase);
                    }
                    break;
                }
            case 2:
                {
                    std::shared_ptr<PlayerMsg_Play> msg = std::make_shared<PlayerMsg_Play>();
                    std::shared_ptr<PlayerMsg_Base> msgBase = std::static_pointer_cast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMDReceiver(msgBase);
                    break;
                }
            case 3:
                {
                    std::shared_ptr<PlayerMsg_Pause> msg = std::make_shared<PlayerMsg_Pause>();
                    std::shared_ptr<PlayerMsg_Base> msgBase = std::static_pointer_cast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMDReceiver(msgBase);
                    break;
                }
            case 4:
                {
                    std::shared_ptr<PlayerMsg_Stop> msg = std::make_shared<PlayerMsg_Stop>();
                    std::shared_ptr<PlayerMsg_Base> msgBase = std::static_pointer_cast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMDReceiver(msgBase);
                    break;
                }
            default:
                break;
        }
    }
    return 0;
}
