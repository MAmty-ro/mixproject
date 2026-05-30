#include <QApplication>
#include "FinestraPrincipale.h"

int main(int argc, char *argv[]) {
    // QApplication gestisce il ciclo di vita del software grafico su Linux
    QApplication app(argc, argv);

    // Creiamo ed esibiamo la nostra plancia di comando
    FinestraPrincipale finestra;
    finestra.show();

    // Fa partire il programma e lo tiene aperto finché non chiudi la finestra premendo la X
    return app.exec();
}