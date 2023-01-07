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
    cycles_t update(DCPU& cpu, Memory& mem) override;
    cycles_t interrupt(DCPU& cpu, Memory& mem) override;

private:
    enum InterruptCommands {
        MEM_MAP_SCREEN = 0,
        MEM_MAP_FONT = 1,
        MEM_MAP_PALETTE = 2,
        SET_BORDER_COLOR = 3,
        MEM_DUMP_FONT = 4,
        MEM_DUMP_PALETTE = 5,
    };
    static constexpr word_t CharWidth = 4;
    static constexpr word_t CharHeight = 8;
    static constexpr word_t Width = 128;
    static constexpr word_t Height = 96;
    static constexpr word_t RowWidth = Width / CharWidth;
    static constexpr word_t PixelZoom = 4;
    static constexpr float BlinkDelay = 0.5f;

    using time = std::chrono::time_point<std::chrono::system_clock>;

    void initializeDefaults();
    void displayChr(word_t data, word_t i, word_t* pixels);
    void dumpFontAtAddress(Memory& mem, word_t addr) const;
    void dumpPalletteAtAddress(Memory& mem, word_t addr) const;
    
    word_t m_memMapAddr = 0;
    word_t m_memFontAddr = 0;
    word_t m_memPaletteAddr = 0;
    word_t m_borderColor = 0;
    time m_blinkTime;
    bool m_blinkSwap = false;

    static const word_t default_font[256];
    word_t m_defaultPalette[16];

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_screenTexture = nullptr;
};
