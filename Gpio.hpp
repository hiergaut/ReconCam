#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

std::string gpioDir;
int sensorGpioNum;
//int sensorAdditional;

int gpioGetValue(int gpio)
{
    std::string gpioValueFile = gpioDir + "gpio" + std::to_string(gpio) + "/value";
    std::ifstream getValueGpio(gpioValueFile.c_str());
    if (!getValueGpio.is_open()) {
        std::cout << "[gpioGetValue] unable to get value gpio" << std::endl;
        perror(gpioValueFile.c_str());
        exit(3);
    }

    int val;
    getValueGpio >> val;
    getValueGpio.close();
    // usleep(100000);

    return val;
}

void gpioSetValue(int gpio, int value)
{
    std::string gpioValueFile = gpioDir + "gpio" + std::to_string(gpio) + "/value";
    std::ofstream setValueGpio(gpioValueFile.c_str());
    if (!setValueGpio.is_open()) {
        std::cout << "unable to set value gpio" << std::endl;
        perror(gpioValueFile.c_str());
        exit(4);
    }

    setValueGpio << value;
    setValueGpio.close();
    // usleep(100000);
}

void initGpio(int gpio, const std::string userDirection)
{
    std::string gpioExportDir = gpioDir + "export";
    std::ofstream exportGpio(gpioExportDir.c_str());
    if (!exportGpio.is_open()) {
        std::cout << "unable to export gpio" << std::endl;
        perror(gpioExportDir.c_str());
        exit(1);
    }
    exportGpio << gpio;
    exportGpio.close();
    usleep(2'000'000);

    std::string gpioNumDir = gpioDir + "gpio" + std::to_string(gpio) + "/";
    std::string gpioNumDirectionDir = gpioNumDir + "direction";
    std::ifstream directionGpioIn(gpioNumDirectionDir.c_str());
    if (!directionGpioIn.is_open()) {
        std::cout << "[initGpio] direction file not exist" << std::endl;
        perror(gpioNumDirectionDir.c_str());
        exit(2);
    }

    std::string curDirection = "";
    directionGpioIn >> curDirection;
    directionGpioIn.close();

    if (userDirection != curDirection) {
        std::ofstream directionGpio(gpioNumDirectionDir.c_str(), std::ios::trunc);
        directionGpio << userDirection;
        if (!directionGpio.is_open()) {
            std::cout << "[initGpio] unable to set direction gpio (" << curDirection << " -> " << userDirection << ")" << std::endl;
            perror(gpioNumDirectionDir.c_str());
            exit(2);
        }
        directionGpio.close();
        usleep(1000000);
    }
}

bool hasMovement()
{
    bool ret = true;

    if (sensorGpioNum != -1) {
        ret = gpioGetValue(sensorGpioNum) == 1;

//        if (sensorAdditional != -1) {
//            ret = ret && gpioGetValue(sensorAdditional) == 1;
//        }
    }

    return ret;
}

bool isNight()
{
    time_t t = time(0);
    tm* now = localtime(&t);

    int hour = now->tm_hour;

    return 19 <= hour && hour <= 7;
}
