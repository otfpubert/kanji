#include <QtWidgets/QApplication>
#include "kanji_main_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Kanji Learning System");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Japanese Learning Tools");
    
    KanjiMainWindow window;
    window.show();
    
    return app.exec();
} 