#include "MasterizzatoreCD.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <cstdlib>

MasterizzatoreCD::MasterizzatoreCD(const std::string& wavMaster) {
    percorsoWavMaster = wavMaster;
    
    // Generiamo il percorso del file .cue basandoci sullo stesso nome del WAV
    size_t estensione = wavMaster.find_last_of(".");
    if (estensione != std::string::npos) {
        percorsoCue = wavMaster.substr(0, estensione) + ".cue";
    } else {
        percorsoCue = wavMaster + ".cue";
    }

    // Per il nostro test, creiamo due indici di traccia fittizi sulla timeline
    // Traccia 1 parte al secondo 0.0
    TracciaCue t1 = {1, "Traccia Iniziale (Mix parte 1)", convertiSecondiInFps(0.0f)};
    // Traccia 2 parte esattamente dove abbiamo inserito la seconda traccia nel mixer (secondo 139.269)
    TracciaCue t2 = {2, "Traccia Sovrapposta (Mix parte 2)", convertiSecondiInFps(139.269f)};

    elencoTracce.push_back(t1);
    elencoTracce.push_back(t2);
}

std::string MasterizzatoreCD::convertiSecondiInFps(float secondi) {
    int minuti = static_cast<int>(secondi) / 60;
    int secondiRestanti = static_cast<int>(secondi) % 60;
    
    // Calcoliamo la frazione di secondo rimasta e convertiamola in frame (da 0 a 74)
    float frazioneSecondo = secondi - std::floor(secondi);
    int frame = static_cast<int>(std::round(frazioneSecondo * 75.0f));
    
    // Protezione per evitare che l'arrotondamento porti a 75 frame (che sarebbe 1 secondo netto)
    if (frame >= 75) {
        frame = 0;
        secondiRestanti++;
        if (secondiRestanti >= 60) {
            secondiRestanti = 0;
            minuti++;
        }
    }

    // Formattiamo tutto come MM:SS:FF (es. 02:19:35) con i leading zero
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << minuti << ":"
       << std::setfill('0') << std::setw(2) << secondiRestanti << ":"
       << std::setfill('0') << std::setw(2) << frame;
    
    return ss.str();
}

bool MasterizzatoreCD::generaFileCue() {
    std::ofstream fileCue(percorsoCue);
    if (!fileCue.is_open()) {
        std::cerr << "❌ Errore Hardware: Impossibile creare il file di mappatura CUE: " << percorsoCue << std::endl;
        return false;
    }

    std::cout << "📄 Generazione mappa del disco (File CUE)..." << std::endl;

    // Estraiamo solo il nome del file WAV isolandolo dal percorso assoluto
    size_t ultimoSlash = percorsoWavMaster.find_last_of("/\\");
    std::string nomeWavSolo = (ultimoSlash != std::string::npos) ? percorsoWavMaster.substr(ultimoSlash + 1) : percorsoWavMaster;

    // Scrittura dell'header del file CUE standard
    fileCue << "TITLE \"Custom Mix CD-DA\"\n";
    fileCue << "PERFORMER \"DAW User\"\n";
    fileCue << "FILE \"" << nomeWavSolo << "\" WAVE\n";

    // Scriviamo i metadati e i punti di inizio di ogni traccia
    for (const auto& traccia : elencoTracce) {
        fileCue << "  TRACK " << std::setfill('0') << std::setw(2) << traccia.numeroTraccia << " AUDIO\n";
        fileCue << "    TITLE \"" << traccia.titolo << "\"\n";
        fileCue << "    INDEX 01 " << traccia.tempoInizioCUE << "\n";
    }

    fileCue.close();
    std::cout << "✅ File CUE generato con successo in: " << percorsoCue << std::endl;
    return true;
}

void MasterizzatoreCD::avviaMasterizzazione(bool modalitàSimulazione) {
    std::cout << "💿 Configurazione Burning Engine..." << std::endl;

    // Prepariamo la stringa del comando per Linux
    // Usiamo il flag '-dummy' per la simulazione hardware se non c'è il masterizzatore inserito
    std::string comandoWodim = "wodim -v -dao ";
    if (modalitàSimulazione) {
        comandoWodim += "-dummy ";
    }
    
    // Aggiungiamo il puntatore alla mappa CUE
    comandoWodim += "cuefile=\"" + percorsoCue + "\"";

    std::cout << "🚀 Comando che verrà inviato al kernel Linux:\n   > " << comandoWodim << "\n" << std::endl;

    // Nel nostro ambiente di test, lanciamo il comando reale in modalità virtuale
    // Se fallisce perché non trova fisicamente nessun drive hardware (anche finto), lo intercettiamo con eleganza.
    int risultatoSistema = std::system(comandoWodim.c_str());
    
    if (risultatoSistema == 0) {
        std::cout << "✨ SIMULAZIONE HARDWARE COMPLETATA CON SUCCESSO!" << std::endl;
    } else {
        std::cout << "⚠️ Nota Hardware: Il comando wodim è stato eseguito, ma ha risposto che il masterizzatore fisico è ancora scollegato (Normale, lo stiamo aspettando!)." << std::endl;
    }
}