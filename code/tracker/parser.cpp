#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

// stuff stolen from milytracker

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
    return (uint16_t)((uint16_t)c[0]+((uint16_t)c[1]<<8));
}

static int mot2int(int x) {
    return (x>>8)+((x&255)<<8);
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "provide a filename" << std::endl;
        return 1;
    }
    std::cout << "opening cympfany.mod" << std::endl;
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("some kind of open error");
        return 1;
    }

    uint8_t block[2048];
    if (read(fd, block, 2048) != 2048) {
        perror("some kind of read error");
        return 1;
    }

    std::cout << "type: " << identifyModule(block) << std::endl;
    lseek(fd, 0, SEEK_SET);

    uint8_t buffer[20];
    if (read(fd, buffer, 20) != 20) {
        perror("some kind of read error");
        return 1;
    }
    std::cout << "got module name: " << buffer << std::endl;

    std::vector<Sample> samples;

    // 31 is for MOD
    for (int i = 0; i < 31; ++i) {
        char samplebuffer[22];
        memset(samplebuffer, 0, 22);
        if (read(fd, samplebuffer, 22) != 22) {
            perror("some kind of read error");
            return 1;
        }
        std::cout << "got sample name: " << samplebuffer << std::endl;

        Sample smp;
        smp.smplen = 2 * mot2int(readWord(fd));
        std::cout << "got smplen: 0x" << std::hex << std::setw(4) << std::setfill('0') << smp.smplen << std::endl;

        smp.name = samplebuffer;

        read(fd, &smp.finetune, 1);
        read(fd, &smp.vol, 1);

        smp.loopstart = 2 * mot2int(readWord(fd));
        std::cout << "got loopstart: 0x" << std::hex << std::setw(4) << std::setfill('0') << smp.loopstart
            << std::endl;

        smp.looplen = 2 * mot2int(readWord(fd));
        std::cout << "got looplen: 0x" << std::hex << std::setw(4) << std::setfill('0') << smp.looplen
            << std::endl << std::endl;

        if (smp.smplen > 2) {
            samples.push_back(smp);
        }
    }
    std::cout << "this module has " << std::dec << samples.size() << " samples" << std::endl;

    uint8_t ordnum;
    read(fd, &ordnum, 1);

    uint8_t whythis;
    read(fd, &whythis, 1);

    uint8_t ord[128];
    read(fd, &ord, 128);

    char sig[5];
    read(fd, &sig, 4);
    sig[4] = '\0';
    std::cout << "sig: x" << sig << "x" << std::endl;

    int patnum = 0;
    for (int i = 0; i < 128; ++i) {
        if (ord[i] > patnum)
            patnum = ord[i];
    }
    ++patnum;

    std::cout << "patnum: " << patnum << std::endl;

    int numChannels = getPTnumchannels(sig);
    std::cout << "numChannels: " << numChannels << std::endl;
    int patternsize = numChannels * 64 * 4;
    std::cout << "patternSize: " << patternsize << std::endl;

    char newbuffer[1024];
    memset(newbuffer, 0, 1024);
    for (int i = 0; i < patnum; ++i) {
        std::cout << std::endl << "pattern " << i << " with size " << patternsize << std::endl;
        read(fd, &newbuffer, patternsize);
        for (int j = 0; j < patternsize; ++j) {
            printf("%02x", newbuffer[j] & 0xFF);
            if ((j+1) % (numChannels / 2) == 0)
                std::cout << " ";
            if ((j+1) % (numChannels * 4) == 0)
                std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    for (unsigned int i = 0; i < samples.size(); ++i) {
        samples[i].sample = new uint8_t[samples[i].smplen];
        memset(samples[i].sample, 0, samples[i].smplen);
        read(fd, samples[i].sample, samples[i].smplen);

        /*
        int fd2 = open("./out.bin", O_RDWR|O_CREAT);
        write(fd2, samples[i].sample, samples[i].smplen);
        close(fd2);
        */
        // break;
        // possible adpcm check
        // omg lets try to play audio
    }

    return 0;
}
