#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "engine.h"
#include <iostream>
#include "Chrono.h"
//Students cannot change anything in this file
#include "ProceduralGeneration.h"

/* Main function */
int main(void) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;

    const double target_frame = 1.0 / 60.0;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    window = SDL_CreateWindow(
        "Framebuffer",
        WIDTH * 4,
        HEIGHT * 4,
        0
    );

    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    renderer = SDL_CreateRenderer(
        window,
        NULL
    );

    if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());

        SDL_DestroyWindow(window);
        SDL_Quit();

        return EXIT_FAILURE;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_XRGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    if (texture == NULL) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return EXIT_FAILURE;
    }

    Engine engine;
    engine.init();
    bool isRunning = true;
    FinalFrameBuffer frameBuffer;
    CHRONO->SetFPSCap(60);

    /* Frame loop */
    while (isRunning) {
        /* Poll events */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
            engine.handleEvent(event);
        }
        engine.Update();
        
        engine.renderFrame(frameBuffer);

        /* Copy contents of the framebuffer to our SDL texture */
        SDL_UpdateTexture(
            texture,
            NULL,
            &frameBuffer.colorData,
            WIDTH * sizeof(uint32_t)
        );

        /* Render texture and display the renderer */
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        CHRONO->Update();
        std::cout << CHRONO->ShowFPS() << std::endl;
    }

    engine.destroy();
    /* Free all the SDL resources we created */
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    CHRONO->Release();

    return EXIT_SUCCESS;
}