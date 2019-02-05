#include <iostream>
#include <string>
#include "MPlayerManager.h"
#include "PlayerMsg_Common.h"
#include "Logger.h"

#include <unistd.h>
#include <memory>

int main(int argc, char* argv[])
{
    MPlayerManager manager;
    manager.InitComponent();
    CmdReceiver* cmdHandler = static_cast<CmdReceiver*>(&manager);
    bool running = true;
    while(running)
    {
        std::cout << "Your action:" << std::endl
                  << "1: Open" << std::endl
                  << "2: Play" << std::endl
                  << "3: Pause" << std::endl
                  << "4: Stop" << std::endl
                  << "5: Debug Toggle" << std::endl
                  << "6: Exit" << std::endl;
        int action; std::string url;
        std::cin >> action;
        switch (action)
        {
            case 1:
                {
                    std::cout << "Input url" << std::endl;
                    std::cin >> url;
                    std::shared_ptr<PlayerMsg_Open> msg = std::make_shared<PlayerMsg_Open>();
                    msg->SetURL(url);
                    std::shared_ptr<PlayerMsg_Base> msgBase = std::static_pointer_cast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMD(msgBase);
                    break;
                }
            case 2:
                {
                    std::shared_ptr<PlayerMsg_Play> msg = std::make_shared<PlayerMsg_Play>();
                    std::shared_ptr<PlayerMsg_Base> msgBase = std::static_pointer_cast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMD(msgBase);
                    break;
                }
            case 3:
                {
                    std::shared_ptr<PlayerMsg_Pause> msg = std::make_shared<PlayerMsg_Pause>();
                    std::shared_ptr<PlayerMsg_Base> msgBase = std::static_pointer_cast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMD(msgBase);
                    break;
                }
            case 4:
                {
                    std::shared_ptr<PlayerMsg_Stop> msg = std::make_shared<PlayerMsg_Stop>();
                    std::shared_ptr<PlayerMsg_Base> msgBase = std::static_pointer_cast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMD(msgBase);
                    break;
                }
            case 5:
                {
                    Logger::LoggerConfig config = Logger::GetInstance().GetConfig();
                    if (config.logLevel ==  Logger::LogLevel::DEBUG)
                        config.logLevel = Logger::LogLevel::WARN;
                    else
                        config.logLevel = Logger::LogLevel::DEBUG;
                    Logger::GetInstance().InitComponent(config);
                    break;
                }
            case 6:
                running = false;
                break;
            default:
                break;
        }
        usleep(100000);
    }
    return 0;
}
