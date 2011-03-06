#include <QApplication>
#include "uxselect.h"

#ifdef USE_PAMHELPER
int pfd[2];
#endif

int main(int argc, char** argv){
#ifdef USE_PAMHELPER
  // we need fd 3 for checkpassword, which we'll most likely not get
  // if we set up the pipe at some later point
  if (pipe(pfd)){
    qDebug() << "pipe() failed";
    exit(-1);
  }
  if (pfd[0]!=3){
    qDebug() << "pfd != 3: " << pfd[0];
    exit(-1);
  }
#endif

  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("aard");
  QCoreApplication::setOrganizationDomain("bwachter.lart.info");
  QCoreApplication::setApplicationName("uxselect");

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);

  QTranslator uxselectTranslator;
  uxselectTranslator.load("uxselect_" + QLocale::system().name());
  app.installTranslator(&uxselectTranslator);

  UxSelect mw;
  mw.show();
  mw.showFullScreen();

  return app.exec();
}
