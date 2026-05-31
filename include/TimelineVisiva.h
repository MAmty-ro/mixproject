#ifndef TIMELINEVISIVA_H
#define TIMELINEVISIVA_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <vector>
#include "AudioClipItem.h"

class TimelineVisiva : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene* scena;
    std::vector<AudioClipItem*> clips;
    float pixelPerSecondo; 
    int numeroTracce;
    float altezzaTraccia;
    void disegnaGrigliaBackground();

public:
    explicit TimelineVisiva(QWidget *parent = nullptr);
    void aggiungiClip(const std::string& percorso, float durata, int tracciaDestinazione);
    void svuotaTimeline();
    std::vector<AudioClipItem*> getClips() const { return clips; }
    float getPixelPerSecondo() const { return pixelPerSecondo; }
};

#endif // TIMELINEVISIVA_H