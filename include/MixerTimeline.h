#ifndef MIXERTIMELINE_H
#define MIXERTIMELINE_H

#include <string>
#include <vector>
#include <cstdint>

struct ElementoTimeline {
    std::string percorsoFile;
    float posizioneTimeline; 
    float offsetInizio;      
    float durataTaglio;      
};

class MixerTimeline {
private:
    std::vector<ElementoTimeline> listaClips;

#pragma pack(push, 1)
    struct WavHeader {
        char chunkId[4] = {'R', 'I', 'F', 'F'};
        uint32_t chunkSize;
        char format[4] = {'W', 'A', 'V', 'E'};
        char subchunk1Id[4] = {'f', 'm', 't', ' '};
        uint32_t subchunk1Size = 16;
        uint16_t audioFormat = 1; 
        uint16_t numChannels = 2; 
        uint32_t sampleRate = 44100;
        uint32_t byteRate = 44100 * 2 * 2;
        uint16_t blockAlign = 2 * 2;
        uint16_t bitsPerSample = 16;
        char subchunk2Id[4] = {'d', 'a', 't', 'a'};
        uint32_t subchunk2Size;
    };
#pragma pack(pop)

public:
    MixerTimeline() = default;
    void aggiungiTracciaVisiva(const std::string& percorso, float posizione, float offsetInizio, float durataTaglio);
    void svuotaTimeline();
    bool esportaMixFinale(const std::string& outputPercorso);
    float getDurataMassimaTimeline() const;
};

#endif // MIXERTIMELINE_H