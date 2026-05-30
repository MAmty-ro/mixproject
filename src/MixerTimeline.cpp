#include "../include/MixerTimeline.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdint>

MixerTimeline::MixerTimeline() {
    durataTotaleMix = 0.0f;
}

void MixerTimeline::aggiungiTraccia(const TracciaAudio& traccia, float tempoInizio, int riga) {
    // Di default impostiamo la transizione su Crossfade
    ElementoTimeline elemento = {traccia, tempoInizio, riga, "Crossfade"};
    canaliTimeline.push_back(elemento);
    
    float fineTraccia = tempoInizio + traccia.getDurata();
    if (fineTraccia > durataTotaleMix) {
        durataTotaleMix = fineTraccia;
    }
}

void MixerTimeline::modificaTransizione(size_t indice, const std::string& tipo) {
    if (indice < canaliTimeline.size()) {
        canaliTimeline[indice].tipoTransizione = tipo;
        
        // Ricalcoliamo la timeline in base al tipo di transizione scelto
        durataTotaleMix = 0.0f;
        float tempoCorrente = 0.0f;
        for (size_t i = 0; i < canaliTimeline.size(); ++i) {
            if (i > 0) {
                if (canaliTimeline[i].tipoTransizione == "Cut Diretto") {
                    // Nessun crossfade, attacca subito dopo la fine della precedente
                    tempoCorrente = durataTotaleMix;
                } else if (canaliTimeline[i].tipoTransizione == "Dissolvenza Lunga") {
                    tempoCorrente = durataTotaleMix - 10.0f; // 10 secondi di mix
                } else {
                    tempoCorrente = durataTotaleMix - 5.0f;  // 5 secondi standard
                }
                if (tempoCorrente < 0.0f) tempoCorrente = 0.0f;
                canaliTimeline[i].secondoInizio = tempoCorrente;
            } else {
                canaliTimeline[i].secondoInizio = 0.0f;
            }
            float fine = canaliTimeline[i].secondoInizio + canaliTimeline[i].audio.getDurata();
            if (fine > durataTotaleMix) durataTotaleMix = fine;
        }
    }
}

void MixerTimeline::svuotaTimeline() {
    canaliTimeline.clear();
    durataTotaleMix = 0.0f;
}

bool MixerTimeline::esportaMixFinale(const std::string& percorsoOutput) {
    if (canaliTimeline.empty()) return false;

    int frequenzaCampionamento = 44100;
    int canali = 2;
    size_t totaleCampioniMix = static_cast<size_t>(durataTotaleMix * frequenzaCampionamento * canali);
    
    std::vector<float> bufferMixMaster(totaleCampioniMix, 0.0f);

    for (const auto& elemento : canaliTimeline) {
        const auto& campioniTraccia = elemento.audio.getCampioni();
        size_t campionInizioMaster = static_cast<size_t>(elemento.secondoInizio * frequenzaCampionamento * canali);
        
        float secondiDissolvenza = 5.0f;
        if (elemento.tipoTransizione == "Dissolvenza Lunga") secondiDissolvenza = 10.0f;
        if (elemento.tipoTransizione == "Cut Diretto") secondiDissolvenza = 0.0f;

        size_t campioniDissolvenza = static_cast<size_t>(secondiDissolvenza * frequenzaCampionamento * canali);
        size_t totaleCampioniTraccia = campioniTraccia.size();

        for (size_t i = 0; i < totaleCampioniTraccia; ++i) {
            size_t indiceMaster = campionInizioMaster + i;
            if (indiceMaster >= bufferMixMaster.size()) break;

            float fattoreVolume = 1.0f;

            // APPLICAZIONE DEI TRE ALGORITMI DI TRANSIZIONE HARDWARE
            if (secondiDissolvenza > 0.0f) {
                // Inviluppo in Ingresso (Fade In)
                if (i < campioniDissolvenza) {
                    float t = static_cast<float>(i) / campioniDissolvenza;
                    if (elemento.tipoTransizione == "Dissolvenza Lunga") {
                        // Curva ad S logaritmica (Smooth Curve)
                        fattoreVolume = 3.0f * t * t - 2.0f * t * t * t;
                    } else {
                        // Dissolvenza lineare standard
                        fattoreVolume = t;
                    }
                }
                // Inviluppo in Uscita (Fade Out)
                else if (i > (totaleCampioniTraccia - campioniDissolvenza)) {
                    size_t campioniDalFondo = totaleCampioniTraccia - i;
                    float t = static_cast<float>(campioniDalFondo) / campioniDissolvenza;
                    if (elemento.tipoTransizione == "Dissolvenza Lunga") {
                        fattoreVolume = 3.0f * t * t - 2.0f * t * t * t;
                    } else {
                        fattoreVolume = t;
                    }
                }
            }

            // Mixaggio matematico lineare con Hard Limiting preventivo
            bufferMixMaster[indiceMaster] += campioniTraccia[i] * fattoreVolume;
            if (bufferMixMaster[indiceMaster] > 1.0f) bufferMixMaster[indiceMaster] = 1.0f;
            if (bufferMixMaster[indiceMaster] < -1.0f) bufferMixMaster[indiceMaster] = -1.0f;
        }
    }

    // Salvataggio nel file WAV fisico a 16-bit Stereo 44100Hz
    std::ofstream fileOut(percorsoOutput, std::ios::binary);
    if (!fileOut.is_open()) return false;

    uint32_t dimensioneDatiAudio = bufferMixMaster.size() * 2;
    uint32_t dimensioneRiff = 36 + dimensioneDatiAudio;

    fileOut.write("RIFF", 4);
    fileOut.write(reinterpret_cast<const char*>(&dimensioneRiff), 4);
    fileOut.write("WAVE", 4);
    fileOut.write("fmt ", 4);
    
    uint32_t subChunkSize = 16;
    uint16_t formatoAudio = 1; 
    uint16_t numCanali = 2;       
    uint32_t campionamento = 44100;
    uint32_t byteAlSecondo = 44100 * 2 * 2; 
    uint16_t allineamentoBlocco = 4;        
    uint16_t bitsPerSample = 16;

    fileOut.write(reinterpret_cast<const char*>(&subChunkSize), 4);
    fileOut.write(reinterpret_cast<const char*>(&formatoAudio), 2);
    fileOut.write(reinterpret_cast<const char*>(&numCanali), 2);
    fileOut.write(reinterpret_cast<const char*>(&campionamento), 4);
    fileOut.write(reinterpret_cast<const char*>(&byteAlSecondo), 4);
    fileOut.write(reinterpret_cast<const char*>(&allineamentoBlocco), 2);
    fileOut.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    fileOut.write("data", 4);
    fileOut.write(reinterpret_cast<const char*>(&dimensioneDatiAudio), 4);

    for (float campione : bufferMixMaster) {
        int16_t pcm16 = static_cast<int16_t>(campione * 32767.0f);
        fileOut.write(reinterpret_cast<const char*>(&pcm16), 2);
    }
    fileOut.close();
    return true;
}