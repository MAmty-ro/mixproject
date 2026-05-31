#include <QApplication>
#include "include/FinestraPrincipale.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    FinestraPrincipale view;
    view.show();
    return app.exec();
}