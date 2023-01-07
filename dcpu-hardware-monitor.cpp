#include <dcpu-hardware-monitor.h>
#include <dcpu-assert.h>
#include <dcpu.h>
#include <dcpu-mem.h>
#include <SDL.h>
#include <cstdio>
// #include <random>
#include <algorithm>

// https://gist.github.com/SylvainBoilard/4645708
const uint16_t Monitor::default_font[] = {
    0xb79e, 0x388e, 0x722c, 0x75f4, 0x19bb, 0x7f8f, 0x85f9, 0xb158,
    0x242e, 0x2400, 0x082a, 0x0800, 0x0008, 0x0000, 0x0808, 0x0808,
    0x00ff, 0x0000, 0x00f8, 0x0808, 0x08f8, 0x0000, 0x080f, 0x0000,
    0x000f, 0x0808, 0x00ff, 0x0808, 0x08f8, 0x0808, 0x08ff, 0x0000,
    0x080f, 0x0808, 0x08ff, 0x0808, 0x6633, 0x99cc, 0x9933, 0x66cc,
    0xfef8, 0xe080, 0x7f1f, 0x0701, 0x0107, 0x1f7f, 0x80e0, 0xf8fe,
    0x5500, 0xaa00, 0x55aa, 0x55aa, 0xffaa, 0xff55, 0x0f0f, 0x0f0f,
    0xf0f0, 0xf0f0, 0x0000, 0xffff, 0xffff, 0x0000, 0xffff, 0xffff,
    0x0000, 0x0000, 0x005f, 0x0000, 0x0300, 0x0300, 0x3e14, 0x3e00,
    0x266b, 0x3200, 0x611c, 0x4300, 0x3629, 0x7650, 0x0002, 0x0100,
    0x1c22, 0x4100, 0x4122, 0x1c00, 0x1408, 0x1400, 0x081c, 0x0800,
    0x4020, 0x0000, 0x0808, 0x0800, 0x0040, 0x0000, 0x601c, 0x0300,
    0x3e49, 0x3e00, 0x427f, 0x4000, 0x6259, 0x4600, 0x2249, 0x3600,
    0x0f08, 0x7f00, 0x2745, 0x3900, 0x3e49, 0x3200, 0x6119, 0x0700,
    0x3649, 0x3600, 0x2649, 0x3e00, 0x0024, 0x0000, 0x4024, 0x0000,
    0x0814, 0x2241, 0x1414, 0x1400, 0x4122, 0x1408, 0x0259, 0x0600,
    0x3e59, 0x5e00, 0x7e09, 0x7e00, 0x7f49, 0x3600, 0x3e41, 0x2200,
    0x7f41, 0x3e00, 0x7f49, 0x4100, 0x7f09, 0x0100, 0x3e41, 0x7a00,
    0x7f08, 0x7f00, 0x417f, 0x4100, 0x2040, 0x3f00, 0x7f08, 0x7700,
    0x7f40, 0x4000, 0x7f06, 0x7f00, 0x7f01, 0x7e00, 0x3e41, 0x3e00,
    0x7f09, 0x0600, 0x3e41, 0xbe00, 0x7f09, 0x7600, 0x2649, 0x3200,
    0x017f, 0x0100, 0x3f40, 0x3f00, 0x1f60, 0x1f00, 0x7f30, 0x7f00,
    0x7708, 0x7700, 0x0778, 0x0700, 0x7149, 0x4700, 0x007f, 0x4100,
    0x031c, 0x6000, 0x0041, 0x7f00, 0x0201, 0x0200, 0x8080, 0x8000,
    0x0001, 0x0200, 0x2454, 0x7800, 0x7f44, 0x3800, 0x3844, 0x2800,
    0x3844, 0x7f00, 0x3854, 0x5800, 0x087e, 0x0900, 0x4854, 0x3c00,
    0x7f04, 0x7800, 0x447d, 0x4000, 0x2040, 0x3d00, 0x7f10, 0x6c00,
    0x417f, 0x4000, 0x7c18, 0x7c00, 0x7c04, 0x7800, 0x3844, 0x3800,
    0x7c14, 0x0800, 0x0814, 0x7c00, 0x7c04, 0x0800, 0x4854, 0x2400,
    0x043e, 0x4400, 0x3c40, 0x7c00, 0x1c60, 0x1c00, 0x7c30, 0x7c00,
    0x6c10, 0x6c00, 0x4c50, 0x3c00, 0x6454, 0x4c00, 0x0836, 0x4100,
    0x0077, 0x0000, 0x4136, 0x0800, 0x0201, 0x0201, 0x0205, 0x0200
};

Monitor::Monitor()
{
    m_id = 0x7349f615;              // LEM1802 - Low Energy Monitor
    m_version = 0x1802;
    m_manifacturer = 0x1c6c8b36;    // NYA_ELEKTRISKA

    initializeDefaults();
    m_blinkTime = std::chrono::system_clock::now();

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    } else {
        m_window = SDL_CreateWindow( "LEM1802 - Low Energy Monitor",
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED,
                                     Width * PixelZoom,
                                     Height * PixelZoom,
                                     SDL_WINDOW_SHOWN );
        if( m_window == nullptr ) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        } else {
            m_renderer = SDL_CreateRenderer( m_window, -1, SDL_RENDERER_ACCELERATED );
            if( m_renderer == nullptr ) {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
            } else {
                SDL_SetRenderDrawColor( m_renderer, 0xFF, 0xFF, 0xFF, 0xFF );

                m_screenTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_STREAMING,
                                                    Width, Height);
                if (m_screenTexture == nullptr) {
                    printf( "texture could not be created! SDL_Error: %s\n", SDL_GetError() );
                } 
            }
        }
    }
}

Monitor::~Monitor() {
    SDL_DestroyTexture( m_screenTexture );
    SDL_DestroyRenderer( m_renderer );
    SDL_DestroyWindow( m_window );
    SDL_Quit();        
}

#define defcolor(r,g,b) ((0xF & r) << 8 | (0xF & g) << 4 | (0xF & b))

void Monitor::initializeDefaults(){
    m_defaultPalette[0]     = defcolor(0xF, 0xF, 0xF);
    m_defaultPalette[1]     = defcolor(0x0, 0x0, 0x0);
    m_defaultPalette[2]     = defcolor(0xF, 0x0, 0x0);
    m_defaultPalette[3]     = defcolor(0x0, 0xF, 0x0);
    m_defaultPalette[4]     = defcolor(0x0, 0x0, 0xF);
    m_defaultPalette[5]     = defcolor(0xF, 0xF, 0x0);
    m_defaultPalette[6]     = defcolor(0x0, 0xF, 0xF);
    m_defaultPalette[7]     = defcolor(0xF, 0x0, 0xF);
    m_defaultPalette[8]     = defcolor(0x7, 0x7, 0x7);
    m_defaultPalette[9]     = defcolor(0x7, 0x0, 0x0);
    m_defaultPalette[10]    = defcolor(0x7, 0x0, 0x0);
    m_defaultPalette[11]    = defcolor(0x0, 0x7, 0x0);
    m_defaultPalette[12]    = defcolor(0x0, 0x0, 0x7);
    m_defaultPalette[13]    = defcolor(0x7, 0x7, 0x0);
    m_defaultPalette[14]    = defcolor(0x0, 0x7, 0x7);
    m_defaultPalette[15]    = defcolor(0x7, 0x0, 0x7);
}

void Monitor::displayChr(uint16_t data, uint16_t i, uint16_t* pixels){
    uint16_t fg = data >> 12;
    uint16_t bg = data == 0 ? 1 : 0xF & (data >> 8); // default pal[1] when no data
    const uint16_t blink = 0x1 & (data >> 7);
    if (blink != 0 && m_blinkSwap) {
        std::swap(fg, bg);
    }
    const uint16_t chr = 0x7F & data;
    const uint16_t fgcolor = (m_defaultPalette[fg] << 4) | 0xF;
    const uint16_t bgcolor = (m_defaultPalette[bg] << 4) | 0xF;
    const uint16_t word0 = default_font[chr*2];
    const uint16_t word1 = default_font[chr*2+1];
    const uint8_t bytes[4] = {static_cast<uint8_t>(word0 >> 8),
                              static_cast<uint8_t>(word0),
                              static_cast<uint8_t>(word1 >> 8),
                              static_cast<uint8_t>(word1)};

    for (int dy=0; dy<8; ++dy) {
        for (int dx=0; dx<4; ++dx) {
            const uint16_t x = CharWidth * (i % RowWidth) + dx;
            const uint16_t y = CharHeight * (i / RowWidth) + dy;

            const bool isForeground = data == 0 ? false : (0x1 & bytes[dx] >> dy); // bgcolor default when no data
            const uint16_t color = isForeground ? fgcolor : bgcolor;
            pixels[x + y*Width] = color;
        }
    }
}

uint32_t Monitor::update(DCPU& cpu, Memory& mem) {
    if (m_memMapAddr == 0)
        return 0;

    if (m_renderer == nullptr || m_screenTexture == nullptr)
        return 0;

    using secduration = std::chrono::duration<float>;
    const time now = std::chrono::system_clock::now();
    const secduration duration = (now - m_blinkTime);
    const float dt = duration.count();
    if (dt > BlinkDelay) {
        m_blinkTime = now;
        m_blinkSwap = !m_blinkSwap;
    }

    uint16_t* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(m_screenTexture, nullptr, (void**)&pixels, &pitch) != 0) {
        printf( "texture could not be locked... SDL_Error: %s\n", SDL_GetError() );
        return 0;
    }

    const uint16_t max = std::min(386, Memory::LastValidAddress+1 - m_memMapAddr);
    for (uint16_t i=0; i<max; ++i) {
        const uint16_t data = mem[m_memMapAddr + i];
        displayChr(data, i, pixels);
    }

    SDL_UnlockTexture(m_screenTexture);
    
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<> distrib(0, 255);
        
    // SDL_LockTexture(m_screenTexture, nullptr, (void**)&pixels, &pitch);
        
    // for (uint32_t i=0; i<Width*Height; ++i) {
    //     pixels[i] = distrib(gen) << 12 | distrib(gen) << 8 | distrib(gen) << 4 | distrib(gen);
    // }

    // SDL_UnlockTexture(m_screenTexture);
        
    SDL_RenderClear( m_renderer );
    SDL_RenderCopy( m_renderer, m_screenTexture, nullptr, nullptr );
    SDL_RenderPresent( m_renderer );
    return 1;
}

uint32_t Monitor::interrupt(DCPU& cpu, Memory& mem) {
    const uint16_t a = cpu.getRegister(Registers_A);
    const uint16_t b = cpu.getRegister(Registers_B);
    uint32_t cycles = 0;
    
    switch (a) {
    case MEM_MAP_SCREEN:
        m_memMapAddr = b;
        // todo implement 1s boot time
        break;
    case MEM_MAP_FONT:
        m_memFontAddr = b;
        break;
    case MEM_MAP_PALETTE:
        m_memPaletteAddr = b;
        break;
    case SET_BORDER_COLOR:
        m_borderColor = b & 0xF;
        break;
    case MEM_DUMP_FONT:
        dumpFontAtAddress(mem, b);
        cycles = 256;
        break;
    case MEM_DUMP_PALETTE:
        dumpPalletteAtAddress(mem, b);
        cycles = 16;
        break;
    default:
        dcpu_assert_fmt(false, "unhandled monitor cmd: %d", a);
    }
    return cycles;
}

void Monitor::dumpFontAtAddress(Memory& mem, uint16_t addr) const{
    for (uint16_t i=0; i<256; ++i) {
        *(mem + addr) = default_font[i];
    }
}

void Monitor::dumpPalletteAtAddress(Memory& mem, uint16_t addr) const{
    for (uint16_t i=0; i<16; ++i) {
        *(mem + addr) = m_defaultPalette[i];
    }
}
