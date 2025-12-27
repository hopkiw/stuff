#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 4096;

SDL_AudioDeviceID gSID = 0;
int gID = 0;

bool Init() {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_EVENTS) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    /*
        int freq;
        SDL_AudioFormat format;
        Uint8 channels;
        Uint8 silence;
        Uint16 samples;
        Uint16 padding;
        Uint32 size;
        SDL_AudioCallback callback;
        void *userdata;
    */

    /*
    SDL_AudioSpec spec;
    spec.freq = 16726;
    spec.format = 8;
    spec.samples = 4096;

    gSID = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    */

    return true;
}

void callback(void *userdata, Uint8 *stream, int len) {
    std::cout << "callback called with len " << len << std::endl;
    int8_t* sstream = reinterpret_cast<int8_t *>(stream);
    len /= sizeof(*sstream);
    const int8_t* buffer = reinterpret_cast<int8_t *>(userdata);
    for (int i = 0; i < len; i++) {
        sstream[i] = buffer[gID++] ^ 0x80;  //  why is this needed lol
    }
}

int main(int argc, char* argv[]) {
    if (!Init()) {
        return 1;
    }

    int fd = open("./out.bin", O_RDONLY);
    if (fd < 0) {
        perror("some kind of open error");
        return 1;
    }

    Uint32 rawLength = 53270;
    char* buffer = new char[rawLength];
    read(fd, buffer, rawLength);

    SDL_AudioSpec rawSpec = {
        .freq = 44100,
        .format = AUDIO_S8,
        .channels = 1,
        .samples = BUFFER_SIZE,
        .callback = callback,
        .userdata = buffer,
    };

    gSID = SDL_OpenAudioDevice(NULL, 0, &rawSpec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);

    SDL_PauseAudioDevice(gSID, 0);
    SDL_Delay(3500);

    return 0;
}
