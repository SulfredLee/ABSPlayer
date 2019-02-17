#include <iostream>
#include <string>
#include "ABSPlayerManager.h"
#include "PlayerMsg_Common.h"
#include "Logger.h"
#include "SmartPointer.h"

#include <unistd.h>

int main(int argc, char* argv[])
{
    ABSPlayerManager manager;
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
                    SmartPointer<PlayerMsg_Open> msg = MakeSmartPointer<PlayerMsg_Open>();
                    msg->SetURL(url);
                    SmartPointer<PlayerMsg_Base> msgBase = StaticCast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMD(msgBase);
                    break;
                }
            case 2:
                {
                    SmartPointer<PlayerMsg_Play> msg = MakeSmartPointer<PlayerMsg_Play>();
                    SmartPointer<PlayerMsg_Base> msgBase = StaticCast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMD(msgBase);
                    break;
                }
            case 3:
                {
                    SmartPointer<PlayerMsg_Pause> msg = MakeSmartPointer<PlayerMsg_Pause>();
                    SmartPointer<PlayerMsg_Base> msgBase = StaticCast<PlayerMsg_Base>(msg);
                    cmdHandler->UpdateCMD(msgBase);
                    break;
                }
            case 4:
                {
                    SmartPointer<PlayerMsg_Stop> msg = MakeSmartPointer<PlayerMsg_Stop>();
                    SmartPointer<PlayerMsg_Base> msgBase = StaticCast<PlayerMsg_Base>(msg);
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
