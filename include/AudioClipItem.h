#ifndef AUDIOCLIPITEM_H
#define AUDIOCLIPITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QObject>
#include <string>

class AudioClipItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT

private:
    std::string percorsoFile;
    std::string nomeFile;
    float durataOriginale; 
    float offsetInizio;    
    float durataTaglio;    
    float posizioneTimeline; 
    int tracciaIndex;      
    bool ridimensionamentoInCorso;
    QGraphicsSimpleTextItem* etichettaTesto;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

public:
    AudioClipItem(const std::string& percorso, float durata, int traccia);
    
    std::string getPercorso() const { return percorsoFile; }
    float getPosizioneTimeline() const { return posizioneTimeline; }
    float getOffsetInizio() const { return offsetInizio; }
    float getDurataTaglio() const { return durataTaglio; }
    int getTracciaIndex() const { return tracciaIndex; }
};

#endif // AUDIOCLIPITEM_H