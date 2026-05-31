#include "../include/MasterizzatoreCD.h"
#include <fstream>
#include <QProcess>
#include <QFileInfo>

MasterizzatoreCD::MasterizzatoreCD(const std::string& percorsoWav) : percorsoWavFinale(percorsoWav) {
    percorsoCueFile = percorsoWav.substr(0, percorsoWav.find_last_of('.')) + ".cue";
}

bool MasterizzatoreCD::generaFileCue() {
    std::ofstream cue(percorsoCueFile);
    if (!cue.is_open()) return false;

    QFileInfo info(QString::fromStdString(percorsoWavFinale));
    cue << "FILE \"" << info.fileName().toStdString() << "\" WAVE\n";
    cue << "  TRACK 01 AUDIO\n";
    cue << "    INDEX 01 00:00:00\n";
    cue.close();
    return true;
}

bool MasterizzatoreCD::avviaMasterizzazione(bool simulazione) {
    QStringList argomenti;
    argomenti << "-v" << "dev=/dev/cdrom" << "-speed=4" << "-dao";
    if (simulazione) argomenti << "-dummy";
    argomenti << QString::fromStdString(percorsoCueFile);

    return QProcess::startDetached("cdrecord", argomenti);
}