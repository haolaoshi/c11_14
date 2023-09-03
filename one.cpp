#include <iostream>  
#include <map>   
#include <fstream>  
#include <sstream>   
#include <string> 
#include <algorithm>  
#include <ctime> 
#include <chrono> 
#include "FileReader.h"
#include "TextToSpeech.h"
#include "Uploading.h"
#include "config.h"
#include "one.h"


bool isCurrentTimeInRange(const std::string& timeRange) {
    std::istringstream iss(timeRange);
    std::string startTime, endTime;

    std::getline(iss, startTime, '-');  // 根据 '-' 分割字符串，保存到 startTime    
    std::getline(iss, endTime, '-');    // 继续根据 '-' 分割字符串，保存到 endTime    

    // 分割 startTime 和 endTime，分别获取小时和分钟    
    int startHour, startMinute, endHour, endMinute;
    int n = sscanf_s(startTime.c_str(), "%d:%d", &startHour, &startMinute);
    int m = sscanf_s(endTime.c_str(), "%d:%d", &endHour, &endMinute);

    if (startHour > endHour) {
        // If the start time is greater than the end time, we're spanning midnight. Adjust the start and end hours accordingly.  
        startHour -= 24;
        endHour -= 24;
    }

    std::time_t now = std::time(nullptr);
    std::tm* currentTime = std::localtime(&now);
    int currentHour = currentTime->tm_hour;
    int currentMinute = currentTime->tm_min;

    if ((currentHour > startHour || (currentHour == startHour && currentMinute >= startMinute))
        && (currentHour < endHour || (currentHour == endHour && currentMinute < endMinute))) {
        std::cout << "["<<startHour << ":" << startMinute << " <= " << currentHour << ":" << currentMinute << " <= " << endHour << ":" << endMinute << "]"<<std::endl;
        return true;
    }
    else {
        return false;
    }
}



void startAllWorkTrheads()
{  
    FileReader::Instance().Start();
    //TextToSpeech::Instance().Start();
    //Uploading::Instance().Start();
}

void stopAllWorkThreads()
{ 
    FileReader::Instance().Stop();
    //TextToSpeech::Instance().Stop();
    //Uploading::Instance().Stop();
}


int main()
{
    Config* config = Config::getInstance();
    auto dictOptions = config->getConfigOptions("timer");

    while (true)
    {
        bool isTimerOut = true;

        for (const auto& option : dictOptions) { 
            if (isCurrentTimeInRange(option.second))
            {
                startAllWorkTrheads(); 
                isTimerOut = false; 
                break;
            }
        }

        if (isTimerOut) {
            stopAllWorkThreads();
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(check_interval_seconds)); 
        
    }
}