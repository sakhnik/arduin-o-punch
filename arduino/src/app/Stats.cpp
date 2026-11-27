#include "Stats.hpp"
#include "OutMux.hpp"
#include <endian.h>
#include <array>
#ifdef ESP32
#include <esp_system.h>

void Stats::ToN()
{
    size = htole16(size);
    bootCount = htole16(size);
    for (int i = 0; i < std::size(timeStats); ++i) {
        timeStats[i] = htole32(timeStats[i]);
    }
}

void Stats::FromN()
{
    size = le16toh(size);
    bootCount = le16toh(bootCount);
    for (int i = 0; i < std::size(timeStats); ++i) {
        timeStats[i] = le32toh(timeStats[i]);
    }
}

void Stats::Print(OutMux &outMux)
{
    outMux.Print("bootCount=");
    outMux.Println(bootCount);
    outMux.Print("lastReset=");
    outMux.Print(lastResetReason);
    outMux.Print(" -> ");
    switch (static_cast<esp_reset_reason_t>(lastResetReason)) {
    case ESP_RST_UNKNOWN: 	outMux.Print("UNKNOWN");    break;
    case ESP_RST_POWERON:   outMux.Print("POWERON");    break;
    case ESP_RST_EXT:       outMux.Print("EXT");        break;
    case ESP_RST_SW:        outMux.Print("SW");         break;
    case ESP_RST_PANIC:     outMux.Print("PANIC");      break;
    case ESP_RST_INT_WDT:   outMux.Print("INT_WDT"); 	break;
    case ESP_RST_TASK_WDT:  outMux.Print("TASK_WDT"); 	break;
    case ESP_RST_WDT:       outMux.Print("WDT");        break;
    case ESP_RST_DEEPSLEEP: outMux.Print("DEEPSLEEP");  break;
    case ESP_RST_BROWNOUT:  outMux.Print("BROWNOUT");   break;
    case ESP_RST_SDIO:      outMux.Print("SDIO");       break;
    }
    outMux.Println();

    DumpTimeStats(outMux);
}

void Stats::DumpTimeStats(OutMux &outMux)
{
    auto toMAH = [](uint32_t s, float current)  {
        return current * s / (60 * 60);
    };

    using Mode = OperationMode;
    auto getCurrent = [](Mode m) -> float {
        switch (m) {
        case Mode::Active: return 20.f;
        case Mode::Eco: return 10.f;
        case Mode::BLE: return 100.f;
        case Mode::WiFi: return 100.f;
        case Mode::Sleep: return 0.05f;
        default: return 0.f;
        }
    };

    uint32_t totalTime = 0;
    float totalEnergy = 0;

    auto printLine = [&](uint32_t time, float current) {
        totalTime += time;
        auto mah = toMAH(time, current);
        totalEnergy += mah;
        char line[64];
        sprintf(line, "%8d (%4d:%02d:%02d) %6.1f mAh", time, time / (60 * 60), (time / 60) % 60, time % 60, mah);
        outMux.Print(line);
    };

    outMux.Print("Active: ");
    printLine(timeStats[static_cast<size_t>(Mode::Active)], 20.f);
    outMux.Println();
    outMux.Print("Eco:    ");
    printLine(timeStats[static_cast<size_t>(Mode::Eco)], 10.f);
    outMux.Println();
    outMux.Print("BLE:    ");
    printLine(timeStats[static_cast<size_t>(Mode::BLE)], 100.f);
    outMux.Println();
    outMux.Print("WiFi:   ");
    printLine(timeStats[static_cast<size_t>(Mode::WiFi)], 100.f);
    outMux.Println();
    outMux.Print("Sleep:  ");
    printLine(timeStats[static_cast<size_t>(Mode::Sleep)], 0.05f);
    outMux.Println();
    auto avgCur = totalEnergy * (60 * 60) / totalTime;
    outMux.Print("Total:  ");
    printLine(totalTime, avgCur);
    outMux.Print("  I ~ ");
    char buf[32];
    sprintf(buf, "%.2f", avgCur);
    outMux.Print(buf);
    outMux.Println(" mA");
}

#endif //ESP32
