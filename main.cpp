#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#define SCREEN_WIDTH 736
#define SCREEN_HEIGHT 496

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

static int quit = 0;

bool green_led = false;
int pos = 0;

int sample_number = 0;

const int AMPLITUDE = 28000;
const int SAMPLE_RATE = 44100;
const int AUDIO_BUFFER_SIZE = 2048;

SDL_AudioSpec audio_spec;

void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes)
{
    Sint16 *buffer = (Sint16*)raw_buffer;
    int length = bytes / 2;

    float notes[] = { 523.2511f, 659.2551f, 783.9909f };
    for (int i = 0; i < length; sample_number++) {
        double time = (double)sample_number / (double)audio_spec.freq;
        float f = 0;
        int n = time * 10;
        if (n < 3) {
            f = notes[n];
            Sint16 sample = (Sint16)(AMPLITUDE * sinf(2.0f * M_PI * f * time));
            buffer[i++] = sample;
            buffer[i++] = sample;
        } else {
            sample_number--;
            buffer[i++] = 0;
            buffer[i++] = 0;
        }
    }
}

void audio_init()
{
    // open audio device
    SDL_AudioSpec want;
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = AUDIO_BUFFER_SIZE;
    want.callback = audio_callback;
    if (SDL_OpenAudio(&want, &audio_spec) != 0) fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
    if (audio_spec.channels != 2) fprintf(stderr, "No stereo audio\n");
    if (audio_spec.format != AUDIO_S16SYS) fprintf(stderr, "Wrong format\n");

    // start playing sound
    SDL_PauseAudio(0); 
}

void play_chord() {
    sample_number = 0;
}

void tick() {
    static int c = 0;
    pos++;
    if (c++ == 10) {
        green_led = !green_led;
        c = 0;
    }
}

// SDL painting functions
void draw_led(int x, int y, int r, int g, int b) {
    SDL_Rect r_scr;
    r_scr.x = x;
    r_scr.y = y;
    r_scr.w = 50;
    r_scr.h = 50;
    SDL_SetRenderDrawColor(renderer, r, g, b, 0x00);
    SDL_RenderFillRect(renderer, &r_scr);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0x00);
    SDL_RenderDrawRect(renderer, &r_scr);
}

void render() {
    // clear background
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    // render LEDs
    int y = 108;
    if (green_led) {
        draw_led(pos, y, 0, 0xff, 0);
    } else {
        draw_led(pos, y, 0, 0x5f, 0);
    }

    // update screen
    SDL_RenderPresent(renderer);
}

#if __EMSCRIPTEN__
void main_tick() {
#else
int main_tick() {
#endif

    SDL_Event event;

#if __EMSCRIPTEN__
    while (SDL_PollEvent(&event)) {
#else
    SDL_WaitEvent(&event);
#endif
        switch (event.type) {
            case SDL_USEREVENT: {
                tick();
                break;
            }

            case SDL_QUIT: {
                quit = 1;
                break;
            }
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        play_chord();
                        break;
                }
                break;
            }
        }
#if __EMSCRIPTEN__
    }
#endif

    render();
    SDL_UpdateWindowSurface(window);

#if !__EMSCRIPTEN__
    return 0;
#endif
}

void main_loop() {
#if __EMSCRIPTEN__
    emscripten_set_main_loop(main_tick, -1, 1);
#else
    while (0 == quit) {
        main_tick();
    }
#endif
}

Uint32 my_callbackfunc(Uint32 interval, void *param) {
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return (interval);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "Audio Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);

    audio_init();

    Uint32 delay = 50;
    SDL_AddTimer(delay, my_callbackfunc, 0);

    main_loop();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
