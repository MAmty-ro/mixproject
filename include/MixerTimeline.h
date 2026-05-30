#ifndef MIXERTIMELINE_H
#define MIXERTIMELINE_H

#include "TracciaAudio.h"
#include <vector>
#include <string>

// Mappa una traccia sulla griglia temporale con la sua transizione specifica
struct ElementoTimeline {
    TracciaAudio audio;
    float secondoInizio;  
    int rigaAssegnata;    
    std::string tipoTransizione; // "Crossfade", "Cut Diretto", "Dissolvenza Lunga"
};

class MixerTimeline {
private:
    std::vector<ElementoTimeline> canaliTimeline;
    float durataTotaleMix;

public:
    MixerTimeline();
    void aggiungiTraccia(const TracciaAudio& traccia, float tempoInizio, int riga);
    void modificaTransizione(size_t indice, const std::string& tipo);
    void svuotaTimeline();

    // IL MOTORE MATEMATICO AGGIORNATO CON GESTIONE TRANSIZIONI REALI
    bool esportaMixFinale(const std::string& percorsoOutput);

    float getDurataTotale() const { return durataTotaleMix; }
    const std::vector<ElementoTimeline>& getCanali() const { return canaliTimeline; }
};

#endif // MIXERTIMELINE_H