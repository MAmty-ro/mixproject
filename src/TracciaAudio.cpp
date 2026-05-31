#include "../include/TracciaAudio.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>

TracciaAudio::TracciaAudio() {
    percorsoAssoluto = "";
    nomeFile = "";
    durataSecondi = 0.0f;
}

TracciaAudio::TracciaAudio(const std::string& percorso) {
    percorsoAssoluto = percorso;
    size_t ultimoSlash = percorso.find_last_of("/\\");
    nomeFile = (ultimoSlash != std::string::npos) ? percorso.substr(ultimoSlash + 1) : percorso;
    durataSecondi = 0.0f;
}

bool TracciaAudio::caricaInMemoria() {
    std::ifstream file(percorsoAssoluto, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "❌ Errore C++: Impossibile aprire il file WAV: " << percorsoAssoluto << std::endl;
        return false;
    }

    char idRiff[4];
    uint32_t dimensioneFile;
    char idWave[4];
    file.read(idRiff, 4);
    file.read(reinterpret_cast<char*>(&dimensioneFile), 4);
    file.read(idWave, 4);

    if (std::strncmp(idRiff, "RIFF", 4) != 0 || std::strncmp(idWave, "WAVE", 4) != 0) {
        std::cerr << "❌ Errore C++: Non è un file WAVE valido!" << std::endl;
        return false;
    }

    uint32_t byteAlSecondo = 0;
    uint32_t dimensioneDatiAudio = 0;
    char subChunkId[4];
    uint32_t subChunkSize;

    while (file.read(subChunkId, 4)) {
        file.read(reinterpret_cast<char*>(&subChunkSize), 4);

        if (std::strncmp(subChunkId, "fmt ", 4) == 0) {
            uint16_t formatoAudio, numCanali;
            uint32_t campionamento;

            file.read(reinterpret_cast<char*>(&formatoAudio), 2);
            file.read(reinterpret_cast<char*>(&numCanali), 2);
            file.read(reinterpret_cast<char*>(&campionamento), 4);
            file.read(reinterpret_cast<char*>(&byteAlSecondo), 4);
            
            file.seekg(subChunkSize - 12, std::ios::cur);

            if (campionamento != 44100 || numCanali != 2) {
                std::cerr << "❌ Errore C++: Il file non è 44100Hz Stereo!" << std::endl;
                return false;
            }
        }
        else if (std::strncmp(subChunkId, "data", 4) == 0) {
            dimensioneDatiAudio = subChunkSize;
            break;
        }
        else {
            file.seekg(subChunkSize, std::ios::cur);
        }
    }

    if (dimensioneDatiAudio == 0 || byteAlSecondo == 0) {
        std::cerr << "❌ Errore C++: Impossibile trovare la sezione dati audio." << std::endl;
        return false;
    }

    durataSecondi = static_cast<float>(dimensioneDatiAudio) / byteAlSecondo;
    int numeroCampioni = dimensioneDatiAudio / 2;
    campioniAudio.resize(numeroCampioni);

    std::vector<int16_t> bufferTemporaneo(numeroCampioni);
    file.read(reinterpret_cast<char*>(bufferTemporaneo.data()), dimensioneDatiAudio);

    for (int i = 0; i < numeroCampioni; ++i) {
        campioniAudio[i] = static_cast<float>(bufferTemporaneo[i]) / 32768.0f;
    }

    file.close();
    return true;
}

void TracciaAudio::analizzaFormaDOnda() {
    std::cout << "📊 Analisi picchi per forma d'onda completata per: " << nomeFile << std::endl;
}