#include "engine.h"
#include <stdlib.h>
#include <SDL3/SDL_events.h>
#include <windows.h>
//Students can change whever they want in this file

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>

#include "stb_image.h"
#include <glm/mat4x4.hpp>

/* Doom fire variables and color palette */
#define FIRE_W WIDTH
#define FIRE_H HEIGHT

static uint8_t firePixels[FIRE_W * FIRE_H];

#include "core.h"

struct PixelData {
    int width, height;
    std::vector<uint32_t> imageData;
};

static PixelData doomFace;

void resetFire()
{
    /* Fire initialization */
    for (int i = 0; i < FIRE_W * FIRE_H; i++) {
        firePixels[i] = 0;
    }

    /* Seed the fire bottom row with maximum intensity (this is the fire source) */
    for (int i = 0; i < FIRE_W; i++) {
        firePixels[(FIRE_H - 1) * FIRE_W + i] = FIRE_PALETTE_SIZE - 1;
    }
}

void Engine::init()
{
    // RANDOM;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    printf("Init engine \n");
    static int nbrChannel;
    unsigned char* data = stbi_load("Assets/doomguy.png", &doomFace.width, &doomFace.height, &nbrChannel, 4);
    
    doomFace.imageData.resize(doomFace.width * doomFace.height);

    //We copy the texture data in a format we can use
    for (int i = 0; i < doomFace.width; ++i)
    {
        for (int j = 0; j < doomFace.height; ++j)
        {
            
            uint32_t colorValue = 0;
            int linearized2D = i + j * doomFace.width;
            colorValue = colorValue | (uint32_t(data[0 +  linearized2D * nbrChannel]) << 16);//First channel is Red, highest weight bits
            colorValue = colorValue | (uint32_t(data[1 +  linearized2D * nbrChannel]) << 8); //Then green
            colorValue = colorValue | (uint32_t(data[2 +  linearized2D * nbrChannel]) << 0); //Finally blue
            doomFace.imageData[linearized2D] = colorValue;
        }
    }
    free(data);
    resetFire();
    m_emitter = new Emitter({50.0f, 50.0f}, {0xFFFFF, 0x070707}, 1000);
    m_emitter->Init();
}

void Engine::Update(float _dt)
{
    if ( m_emitter )
        m_emitter->Update(_dt);
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        if (event.key.key == SDLK_SPACE)
        {
            resetFire();
        }
        
        if ( event.key.key == SDLK_P )
        {
            m_emitter->m_active = !m_emitter->m_active;
        }
    }
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{

    frameBufferOut.clear(0x000000);

    /* Fire update */
    for (uint32_t x = 0; x < FIRE_W; x++) {
        for (uint32_t y = 1; y < FIRE_H; y++) {
            uint32_t src = FIRE_W * y + x;
            uint32_t rand_idx = rand() % 4;
            int32_t dst = (int32_t)src - FIRE_W;
            int32_t dst_x = (int32_t)(src % FIRE_W) - (int32_t)rand_idx + 1;
            if (dst < 0 || dst_x < 0 || dst_x >= FIRE_W) {
                continue;
            } else {
                dst = dst - (int32_t)(src % FIRE_W) + dst_x;
                uint8_t src_val = firePixels[src];
                uint8_t decay = rand_idx & 1;
                firePixels[dst] = (src_val > decay) ? (src_val - decay) : 0;
            }
        }
    }

    //draws the doomguy face texture at the center of the screen
    for (int x = 0; x < doomFace.width; x++)
    {
        for (int y = 0; y < doomFace.height; y++) 
        {
            int posX = (WIDTH / 2 - doomFace.width/2) + x;
            int posY = (HEIGHT / 2 - doomFace.height/2) + y;
            frameBufferOut.put_pixel(posX, posY, doomFace.imageData[x + y*doomFace.width]);
        }
    }
    /* Fire render */
    for (int y = 0; y < FIRE_H; y++) {
        for (int x = 0; x < FIRE_W; x++) {
            uint8_t idx = firePixels[y * FIRE_W + x];
            if(idx != 0)
                frameBufferOut.put_pixel(x, y, fire_palette[idx]);
        }
    }
    
    m_emitter->Render(frameBufferOut);
}

void Engine::destroy()
{
    doomFace.imageData = std::vector<uint32_t>();
    printf("Killing engine \n");
}
