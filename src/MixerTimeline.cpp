#include "MixerTimeline.h"
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
    ElementoTimeline elemento = {traccia, tempoInizio, riga};
    canaliTimeline.push_back(elemento);
    
    // Ricalcoliamo la durata totale della timeline
    float fineTraccia = tempoInizio + traccia.getDurata();
    if (fineTraccia > durataTotaleMix) {
        durataTotaleMix = fineTraccia;
    }
    std::cout << "📌 Traccia '" << traccia.getNomeFile() << "' aggiunta al secondo " << tempoInizio << " sulla riga " << riga << std::endl;
}

void MixerTimeline::svuotaTimeline() {
    canaliTimeline.clear();
    durataTotaleMix = 0.0f;
}

bool MixerTimeline::esportaMixFinale(const std::string& percorsoOutput, float secondiDissolvenza) {
    if (canaliTimeline.empty()) {
        std::cerr << "❌ Errore Mixer: Timeline vuota. Impossibile esportare." << std::endl;
        return false;
    }

    std::cout << "🎛️ Avvio mixaggio hardware. Durata totale stimata: " << durataTotaleMix << " secondi." << std::endl;

    // 1. Allochiamo il buffer del mix finale in RAM
    // 44100 Hz * 2 canali (Stereo) = 88200 campioni per ogni secondo di musica
    long long totaleCampioniMix = static_cast<long long>(durataTotaleMix * 44100.0f * 2.0f);
    std::vector<float> bufferMixFinale(totaleCampioniMix, 0.0f);

    // 2. Fondiamo le tracce nel buffer (Algoritmo di Mixdown)
    for (const auto& elemento : canaliTimeline) {
        const auto& campioniTraccia = elemento.audio.getCampioni();
        long long campionInizioTimeline = static_cast<long long>(elemento.secondoInizio * 44100.0f * 2.0f);
        
        // Assicuriamoci che l'indice rimanga pari (inizio canale Sinistro)
        if (campionInizioTimeline % 2 != 0) campionInizioTimeline--;

        long long durataCampioniTraccia = campioniTraccia.size();
        long long campioniDissolvenza = static_cast<long long>(secondiDissolvenza * 44100.0f * 2.0f);

        for (long long i = 0; i < durataCampioniTraccia; ++i) {
            long long indiceDestinazione = campionInizioTimeline + i;
            
            // Protezione da buffer overflow della timeline
            if (indiceDestinazione >= totaleCampioniMix) break;

            // --- ALGORITMO DI DISSOLVENZA (CROSSFADE) ---
            float fattoreVolume = 1.0f;

            // Dissolvenza in entrata (Fade-In) se la traccia è all'inizio
            if (i < campioniDissolvenza && elemento.secondoInizio > 0) {
                fattoreVolume = static_cast<float>(i) / campioniDissolvenza;
            }
            // Dissolvenza in uscita (Fade-Out) se siamo alla fine della traccia
            else if (i > (durataCampioniTraccia - campioniDissolvenza)) {
                long long campioniRimasti = durataCampioniTraccia - i;
                fattoreVolume = static_cast<float>(campioniRimasti) / campioniDissolvenza;
            }

            // Somma algebrica dei campioni audio moltiplicati per il volume della dissolvenza
            bufferMixFinale[indiceDestinazione] += campioniTraccia[i] * fattoreVolume;
        }
    }

    // 3. HARD LIMITER (Prevenzione del Clipping)
    // Se sommiamo due tracce a volume massimo, il segnale supera 1.0 o scende sotto -1.0,
    // creando distorsione digitale (un rumore orribile). Questo ciclo "schiaccia" l'audio entro i limiti di sicurezza.
    for (long long i = 0; i < totaleCampioniMix; ++i) {
        if (bufferMixFinale[i] > 1.0f) bufferMixFinale[i] = 1.0f;
        if (bufferMixFinale[i] < -1.0f) bufferMixFinale[i] = -1.0f;
    }

    // 4. SCRITTURA FILE WAV FINALE SU DISCO
    std::ofstream fileOut(percorsoOutput, std::ios::binary);
    if (!fileOut.is_open()) {
        std::cerr << "❌ Errore Mixer: Impossibile creare il file di output." << std::endl;
        return false;
    }

    // Scriviamo l'header standard a 44 byte
    uint32_t dimensioneDatiAudio = totaleCampioniMix * 2; // 2 byte per campione (16-bit)
    uint32_t dimensioneFileSenzaRiff = 36 + dimensioneDatiAudio;

    fileOut.write("RIFF", 4);
    fileOut.write(reinterpret_cast<const char*>(&dimensioneFileSenzaRiff), 4);
    fileOut.write("WAVE", 4);
    fileOut.write("fmt ", 4);
    
    uint32_t subChunkSize = 16;
    uint16_t formatoAudio = 1; // PCM
    uint16_t canali = 2;       // Stereo
    uint32_t campionamento = 44100;
    uint32_t byteAlSecondo = 44100 * 2 * 2; // SampleRate * NumChannels * (BitsPerSample/8)
    uint16_t allineamentoBlocco = 4;        // NumChannels * (BitsPerSample/8)
    uint16_t bitsPerSample = 16;

    fileOut.write(reinterpret_cast<const char*>(&subChunkSize), 4);
    fileOut.write(reinterpret_cast<const char*>(&formatoAudio), 2);
    fileOut.write(reinterpret_cast<const char*>(&canali), 2);
    fileOut.write(reinterpret_cast<const char*>(&campionamento), 4);
    fileOut.write(reinterpret_cast<const char*>(&byteAlSecondo), 4);
    fileOut.write(reinterpret_cast<const char*>(&allineamentoBlocco), 2);
    fileOut.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    
    fileOut.write("data", 4);
    fileOut.write(reinterpret_cast<const char*>(&dimensioneDatiAudio), 4);

    // Convertiamo i float (-1.0 a 1.0) di nuovo in interi a 16-bit (da -32768 a 32767) per salvarli nel file finale
    std::vector<int16_t> bufferOutput16Bit(totaleCampioniMix);
    for (long long i = 0; i < totaleCampioniMix; ++i) {
        bufferOutput16Bit[i] = static_cast<int16_t>(bufferMixFinale[i] * 32767.0f);
    }

    fileOut.write(reinterpret_cast<const char*>(bufferOutput16Bit.data()), dimensioneDatiAudio);
    fileOut.close();

    std::cout << "💾 Esportazione completata con successo! File creato: " << percorsoOutput << std::endl;
    return true;
}