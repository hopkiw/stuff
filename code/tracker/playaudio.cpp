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


bool Init() {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_EVENTS) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
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

    int rawLength = 53270;
    char* buffer = new char[rawLength];
    read(fd, buffer, rawLength);

    for (int i = 0; i < rawLength; ++i)
        buffer[i] ^= 0x80;

    SDL_AudioSpec rawSpec = {
        .freq = 44100,
        .format = AUDIO_S8,
        .channels = 1,
        .samples = BUFFER_SIZE,
    };

    SDL_AudioDeviceID gSID = SDL_OpenAudioDevice(NULL, 0, &rawSpec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    SDL_QueueAudio(gSID, buffer, rawLength);
    SDL_PauseAudioDevice(gSID, 0);
    while (SDL_GetQueuedAudioSize(gSID) != 0)
        SDL_Delay(30);
    std::cout << "all done." << std::endl;

    return 0;
}
