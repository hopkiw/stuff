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
SDL_AudioDeviceID gSID;

bool Init() {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_EVENTS) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_AudioSpec rawSpec = {
        .freq = SAMPLE_RATE,
        .format = AUDIO_S8,
        .channels = 1,
        .samples = BUFFER_SIZE,
    };

    gSID = SDL_OpenAudioDevice(NULL, 0, &rawSpec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);

    if (gSID == 0)
        return false;

    return true;
}


// stuff stolen from milkytracker

static int getPTnumchannels(const char *id) {
    if (!memcmp(id, "M.K.", 4) || !memcmp(id, "M!K!", 4) || !memcmp(id, "FLT4", 4)) {
        return 4;
    }
    if (!memcmp(id, "FLT8", 4) || !memcmp(id, "OKTA", 4) || !memcmp(id, "OCTA", 4) || !memcmp(id, "FA08", 4)
            || !memcmp(id, "CD81", 4)) {
        return 8;
    }
    if (id[0] >= '1' && id[0] <= '9' && !memcmp(id + 1, "CHN", 3)) {
        return id[0] - '0';
    }
    if (id[0] >= '1' && id[0] <= '9' && id[1] >='0' && id[1] <= '9' && (!memcmp(id + 2, "CH", 2)
                || !memcmp(id + 2, "CN", 2))) {
        return (id[0] - '0') * 10 + id[1] - '0';
    }

    return 0;
}

const char* identifyModule(uint8_t* buffer) {
    // check for .MOD
    if (getPTnumchannels(reinterpret_cast<char*>(buffer) + 1080)) {
        return "MOD";
    }

    int i, j;
    uint8_t* uBuffer = buffer;

    // see if we're getting a song title
    for (i = 0; i < 20; i++)
        if (uBuffer[i] >= 126 || (uBuffer[i] < 32 && uBuffer[i]))
            return NULL;
    uBuffer+=20;

    int lastAsciiValues = -1;
    for (j = 0; j < 15; j++) {
        if (uBuffer[24])
            break;

        if (uBuffer[25] > 64)
            break;

        bool ascii = true;
        for (i = 0; i < 22; i++) {
            if (uBuffer[i] >= 126 || (uBuffer[i] < 14 && uBuffer[i])) {
                ascii = false;
                break;
            }
        }

        if (ascii)
            lastAsciiValues = j;
        else
            break;

        uBuffer+=30;
    }

    if (lastAsciiValues != 14)
        return NULL;

    if (!*uBuffer || *uBuffer > 128)
        return NULL;

    *uBuffer+=2;

    for (i = 0; i < 128; i++)
        if (uBuffer[i] > 128)
            return NULL;

    return "M15";
}


uint16_t readWord(int fd) {
    char c[2];
    if (read(fd, c, 2) != 2) {
        perror("some kind of read error");
        return 1;
    }
    return (uint16_t)((uint16_t)c[0] + ((uint16_t)c[1] << 8));
}

static int mot2int(int x) {
    return (x >> 8) + ((x & 255) << 8);
}

typedef struct Sample {
    std::string name;
    uint8_t* sample = NULL;
    int smplen;
    int finetune;
    int vol;
    int looplen;
    int loopstart;
} Sample;

void playSample(Sample* smp) {
    for (int i = 0; i < smp->smplen; ++i)
        smp->sample[i] ^= 0x80;
    SDL_QueueAudio(gSID, smp->sample, smp->smplen);
    SDL_PauseAudioDevice(gSID, 0);
    while (SDL_GetQueuedAudioSize(gSID) != 0)
        SDL_Delay(30);
    SDL_PauseAudioDevice(gSID, 1);
    SDL_Delay(30);
}

class Module {
 public:
     Module() {}
     bool Parse(const std::string&);

 //private:
     std::vector<Sample> samples;
     std::vector<char*> patterns;
};

bool Module::Parse(const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) {
        perror("some kind of open error");
        return false;
    }

    uint8_t block[2048];
    if (read(fd, block, 2048) != 2048) {
        perror("some kind of read error");
        return false;
    }
    lseek(fd, 0, SEEK_SET);

    uint8_t namebuffer[20];
    if (read(fd, namebuffer, 20) != 20) {
        return false;
    }

    // 31 is for MOD
    for (int i = 0; i < 31; ++i) {
        char samplebuffer[22];
        memset(samplebuffer, 0, 22);
        if (read(fd, samplebuffer, 22) != 22) {
            return false;
        }

        Sample smp;
        smp.smplen = 2 * mot2int(readWord(fd));
        smp.name = samplebuffer;

        read(fd, &smp.finetune, 1);
        read(fd, &smp.vol, 1);

        smp.loopstart = 2 * mot2int(readWord(fd));
        smp.looplen = 2 * mot2int(readWord(fd));

        if (smp.smplen > 2) {
            samples.push_back(smp);
        }
    }

    uint8_t ordnum;
    read(fd, &ordnum, 1);

    uint8_t whythis;
    read(fd, &whythis, 1);

    uint8_t ord[128];
    read(fd, &ord, 128);

    char sig[5];
    read(fd, &sig, 4);
    sig[4] = '\0';

    int patnum = 0;
    for (int i = 0; i < 128; ++i) {
        if (ord[i] > patnum)
            patnum = ord[i];
    }
    ++patnum;

    int numChannels = getPTnumchannels(sig);
    int patternsize = numChannels * 64 * 4;

    for (int i = 0; i < patnum; ++i) {
        char* buffer = new char[patternsize];
        memset(buffer, 0, patternsize);
        if (read(fd, buffer, patternsize) != patternsize) {
            return false;
        }
        patterns.push_back(buffer);
    }

    for (unsigned int i = 0; i < samples.size(); ++i) {
        samples[i].sample = new uint8_t[samples[i].smplen];
        memset(samples[i].sample, 0, samples[i].smplen);
        if (read(fd, samples[i].sample, samples[i].smplen) != samples[i].smplen) {
            return false;
        }
    }


    return true;
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cout << "provide a filename." << std::endl;
        return 1;
    }

    if (!Init()) {
        std::cout << "couldn't initialize SDL" << std::endl;
        return 1;
    }

    Module mod;
    mod.Parse(argv[1]);

    for (size_t i = 0; i < mod.samples.size(); ++i) {
        std::cout << "playing " << mod.samples[i].name << std::endl;
        playSample(&mod.samples[i]);
    }

    std::cout << "all done." << std::endl;
    return 0;
}
