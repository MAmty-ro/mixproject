#ifndef MIXERTIMELINE_H
#define MIXERTIMELINE_H

#include "TracciaAudio.h"
#include <vector>
#include <string>

// Struttura che mappa una traccia sulla griglia temporale
struct ElementoTimeline {
    TracciaAudio audio;
    float secondoInizio;  // Il punto esatto della timeline in cui inizia (es. al secondo 10.5)
    int rigaAssegnata;    // Track 1, Track 2, Track 3...
};

class MixerTimeline {
private:
    std::vector<ElementoTimeline> canaliTimeline;
    float durataTotaleMix;

public:
    MixerTimeline();

    // Aggiunge una traccia posizionandola nel tempo
    void aggiungiTraccia(const TracciaAudio& traccia, float tempoInizio, int riga);
    
    // Svuota la timeline
    void svuotaTimeline();

    // IL MOTORE MATEMATICO: Prende tutte le tracce, calcola le sovrapposizioni,
    // applica le dissolvenze e sputa fuori il file WAV gigante per il CD.
    bool esportaMixFinale(const std::string& percorsoOutput, float secondiDissolvenza);

    // Getter
    float getDurataTotale() const { return durataTotaleMix; }
};

#endif // MIXERTIMELINE_H