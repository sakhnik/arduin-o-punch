#include "Stats.h"
#include "OutMux.h"
#include <endian.h>

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
    outMux.print("bootCount=");
    outMux.println(bootCount);
    outMux.print("lastReset=");
    outMux.print(lastResetReason);
    outMux.print(" -> ");
    switch (static_cast<esp_reset_reason_t>(lastResetReason)) {
    case ESP_RST_UNKNOWN: 	outMux.print("UNKNOWN");    break;
    case ESP_RST_POWERON:   outMux.print("POWERON");    break;
    case ESP_RST_EXT:       outMux.print("EXT");        break;
    case ESP_RST_SW:        outMux.print("SW");         break;
    case ESP_RST_PANIC:     outMux.print("PANIC");      break;
    case ESP_RST_INT_WDT:   outMux.print("INT_WDT"); 	break;
    case ESP_RST_TASK_WDT:  outMux.print("TASK_WDT"); 	break;
    case ESP_RST_WDT:       outMux.print("WDT");        break;
    case ESP_RST_DEEPSLEEP: outMux.print("DEEPSLEEP");  break;
    case ESP_RST_BROWNOUT:  outMux.print("BROWNOUT");   break;
    case ESP_RST_SDIO:      outMux.print("SDIO");       break;
    }
    outMux.println();

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
        outMux.print(line);
    };

    outMux.print("Active: ");
    printLine(timeStats[static_cast<size_t>(Mode::Active)], 20.f);
    outMux.println();
    outMux.print("Eco:    ");
    printLine(timeStats[static_cast<size_t>(Mode::Eco)], 10.f);
    outMux.println();
    outMux.print("BLE:    ");
    printLine(timeStats[static_cast<size_t>(Mode::BLE)], 100.f);
    outMux.println();
    outMux.print("WiFi:   ");
    printLine(timeStats[static_cast<size_t>(Mode::WiFi)], 100.f);
    outMux.println();
    outMux.print("Sleep:  ");
    printLine(timeStats[static_cast<size_t>(Mode::Sleep)], 0.05f);
    outMux.println();
    auto avgCur = totalEnergy * (60 * 60) / totalTime;
    outMux.print("Total:  ");
    printLine(totalTime, avgCur);
    outMux.print("  I ~ ");
    char buf[32];
    sprintf(buf, "%.2f", avgCur);
    outMux.print(buf);
    outMux.println(" mA");
}
