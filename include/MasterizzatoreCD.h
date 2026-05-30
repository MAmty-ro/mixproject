#ifndef MASTERIZZATORECD_H
#define MASTERIZZATORECD_H

#include <string>
#include <vector>

// Struttura per definire i punti di inizio delle tracce nel file CUE
struct TracciaCue {
    int numeroTraccia;
    std::string titolo;
    std::string tempoInizioCUE; // Formato MM:SS:FF (Minuti:Secondi:Frames)
};

class MasterizzatoreCD {
private:
    std::string percorsoWavMaster;
    std::string percorsoCue;
    std::vector<TracciaCue> elencoTracce;

    // Funzione interna per convertire i secondi nel formato Red Book (75 frame al secondo)
    std::string convertiSecondiInFps(float secondi);

public:
    MasterizzatoreCD(const std::string& wavMaster);

    // Genera il file .cue necessario per la masterizzazione DAO
    bool generaFileCue();

    // Avvia la simulazione o la scrittura fisica usando wodim
    void avviaMasterizzazione(bool modalitàSimulazione);
};

#endif // MASTERIZZATORECD_H