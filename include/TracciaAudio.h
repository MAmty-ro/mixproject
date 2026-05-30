#ifndef TRACCIAAUDIO_H
#define TRACCIAAUDIO_H

#include <string>
#include <vector>

class TracciaAudio {
private:
    std::string nomeFile;
    std::string percorsoAssoluto;
    float durataSecondi;
    std::vector<float> campioniAudio;

public:
    TracciaAudio(const std::string& percorso);
    bool caricaInMemoria();
    void analizzaFormaDOnda();

    std::string getNomeFile() const { return nomeFile; }
    std::string getPercorso() const { return percorsoAssoluto; }
    float getDurata() const { return durataSecondi; }
    const std::vector<float>& getCampioni() const { return campioniAudio; }
};

#endif // TRACCIAAUDIO_H