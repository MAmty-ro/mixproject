#ifndef MASTERIZZATORECD_H
#define MASTERIZZATORECD_H

#include <string>

class MasterizzatoreCD {
private:
    std::string percorsoWavFinale;
    std::string percorsoCueFile;

public:
    explicit MasterizzatoreCD(const std::string& percorsoWav);
    bool generaFileCue();
    bool avviaMasterizzazione(bool simulazione = true);
};

#endif // MASTERIZZATORECD_H