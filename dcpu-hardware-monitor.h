#pragma once
#include <dcpu-hardware.h>
#include <chrono>
#include <cstdint>

class SDL_Renderer;
class SDL_Texture;
class SDL_Window;

class Monitor : public Hardware {
public:
    Monitor();
    ~Monitor() override;
    uint32_t update(DCPU& cpu, Memory& mem) override;
    uint32_t interrupt(DCPU& cpu, Memory& mem) override;

private:
    enum InterruptCommands {
        MEM_MAP_SCREEN = 0,
        MEM_MAP_FONT = 1,
        MEM_MAP_PALETTE = 2,
        SET_BORDER_COLOR = 3,
        MEM_DUMP_FONT = 4,
        MEM_DUMP_PALETTE = 5,
    };
    static constexpr uint16_t CharWidth = 4;
    static constexpr uint16_t CharHeight = 8;
    static constexpr uint16_t Width = 128;
    static constexpr uint16_t Height = 96;
    static constexpr uint16_t RowWidth = Width / CharWidth;

    void initializeDefaults();
    void displayChr(uint16_t data, uint16_t i, uint16_t* pixels);
    void dumpFontAtAddress(Memory& mem, uint16_t addr) const;
    void dumpPalletteAtAddress(Memory& mem, uint16_t addr) const;
    
    uint16_t m_memMapAddr = 0;
    uint16_t m_memFontAddr = 0;
    uint16_t m_memPaletteAddr = 0;
    uint16_t m_borderColor = 0;

    static const uint16_t default_font[256];
    uint16_t m_defaultPalette[16];

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_screenTexture = nullptr;
};
