#ifndef TRACCIAAUDIO_H
#define TRACCIAAUDIO_H

#include <string>
#include <vector>

class TracciaAudio {
private:
    std::string percorsoAssoluto;
    std::string nomeFile;
    float durataSecondi;
    std::vector<float> campioniAudio;

public:
    TracciaAudio();
    explicit TracciaAudio(const std::string& percorso);
    
    bool caricaInMemoria();
    void analizzaFormaDOnda();

    std::string getPercorso() const { return percorsoAssoluto; }
    std::string getNomeFile() const { return nomeFile; }
    float getDurata() const { return durataSecondi; } 
    const std::vector<float>& getCampioni() const { return campioniAudio; }
};

#endif // TRACCIAAUDIO_H