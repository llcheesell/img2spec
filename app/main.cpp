#include <QApplication>
#include "app/MainWindow.h"
#include <iostream>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    std::cout << "=== img2spec - Image to Spectrogram Audio Generator ===" << std::endl;
    std::cout << "Starting application..." << std::endl;

    img2spec::MainWindow window;
    window.show();

    return app.exec();
}
