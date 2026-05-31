#include "../include/MixerTimeline.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstring>

void MixerTimeline::aggiungiTracciaVisiva(const std::string& percorso, float posizione, float offsetInizio, float durataTaglio) {
    ElementoTimeline clip;
    clip.percorsoFile = percorso;
    clip.posizioneTimeline = posizione;
    clip.offsetInizio = offsetInizio;
    clip.durataTaglio = durataTaglio;
    listaClips.push_back(clip);
}

void MixerTimeline::svuotaTimeline() {
    listaClips.clear();
}

float MixerTimeline::getDurataMassimaTimeline() const {
    float maxDurata = 0.0f;
    for (const auto& clip : listaClips) {
        float fineClip = clip.posizioneTimeline + clip.durataTaglio;
        if (fineClip > maxDurata) maxDurata = fineClip;
    }
    return maxDurata;
}

bool MixerTimeline::esportaMixFinale(const std::string& outputPercorso) {
    float durataTotaleS = getDurataMassimaTimeline();
    if (durataTotaleS <= 0.0f) return false;

    uint32_t sampleRate = 44100;
    uint32_t canali = 2;
    uint64_t totaleCampioniMaster = static_cast<uint64_t>(durataTotaleS * sampleRate);
    
    std::vector<float> bufferMasterL(totaleCampioniMaster, 0.0f);
    std::vector<float> bufferMasterR(totaleCampioniMaster, 0.0f);

    for (const auto& clip : listaClips) {
        std::ifstream fileWav(clip.percorsoFile, std::ios::binary);
        if (!fileWav.is_open()) continue;

        char idRiff[4], idWave[4];
        uint32_t dimensioneFile;
        fileWav.read(idRiff, 4);
        fileWav.read(reinterpret_cast<char*>(&dimensioneFile), 4);
        fileWav.read(idWave, 4);

        if (std::strncmp(idRiff, "RIFF", 4) != 0 || std::strncmp(idWave, "WAVE", 4) != 0) {
            fileWav.close();
            continue;
        }

        char subChunkId[4];
        uint32_t subChunkSize;
        uint64_t posizioneDataChunk = 0;
        uint32_t byteAlSecondo = 44100 * 2 * 2; 

        while (fileWav.read(subChunkId, 4)) {
            fileWav.read(reinterpret_cast<char*>(&subChunkSize), 4);
            if (std::strncmp(subChunkId, "fmt ", 4) == 0) {
                fileWav.seekg(8, std::ios::cur); 
                fileWav.read(reinterpret_cast<char*>(&byteAlSecondo), 4);
                fileWav.seekg(subChunkSize - 12, std::ios::cur);
            } 
            else if (std::strncmp(subChunkId, "data", 4) == 0) {
                posizioneDataChunk = fileWav.tellg();
                break;
            } 
            else {
                fileWav.seekg(subChunkSize, std::ios::cur);
            }
        }

        if (posizioneDataChunk == 0) {
            fileWav.close();
            continue;
        }

        uint64_t byteOffsetInizio = static_cast<uint64_t>(clip.offsetInizio * byteAlSecondo);
        // Allineamento al blocco da 4 byte (2 canali * 2 byte)
        byteOffsetInizio = (byteOffsetInizio / 4) * 4; 
        
        fileWav.seekg(posizioneDataChunk + byteOffsetInizio, std::ios::beg);

        uint64_t campioneInizioTimeline = static_cast<uint64_t>(clip.posizioneTimeline * sampleRate);
        uint64_t campioniDaLeggere = static_cast<uint64_t>(clip.durataTaglio * sampleRate);

        for (uint64_t i = 0; i < campioniDaLeggere; ++i) {
            if (campioneInizioTimeline + i >= totaleCampioniMaster) break;

            int16_t sampleL = 0, sampleR = 0;
            fileWav.read(reinterpret_cast<char*>(&sampleL), 2);
            fileWav.read(reinterpret_cast<char*>(&sampleR), 2);
            if (fileWav.gcount() < 4) break; 

            bufferMasterL[campioneInizioTimeline + i] += static_cast<float>(sampleL) / 32768.0f;
            bufferMasterR[campioneInizioTimeline + i] += static_cast<float>(sampleR) / 32768.0f;
        }
        fileWav.close();
    }

    std::ofstream fileOut(outputPercorso, std::ios::binary);
    if (!fileOut.is_open()) return false;

    WavHeader header;
    uint32_t dataSizeBytes = totaleCampioniMaster * canali * 2;
    header.subchunk2Size = dataSizeBytes;
    header.chunkSize = 36 + dataSizeBytes;

    fileOut.write(reinterpret_cast<char*>(&header), sizeof(WavHeader));

    for (uint64_t i = 0; i < totaleCampioniMaster; ++i) {
        float fL = std::max(-1.0f, std::min(1.0f, bufferMasterL[i]));
        float fR = std::max(-1.0f, std::min(1.0f, bufferMasterR[i]));

        int16_t outL = static_cast<int16_t>(fL * 32767.0f);
        int16_t outR = static_cast<int16_t>(fR * 32767.0f);

        fileOut.write(reinterpret_cast<char*>(&outL), 2);
        fileOut.write(reinterpret_cast<char*>(&outR), 2);
    }
    fileOut.close();
    return true;
}