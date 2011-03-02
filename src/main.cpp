#include <QApplication>
#include "uxselect.h"

int main(int argc, char** argv){

  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("LART");
  QCoreApplication::setOrganizationDomain("lart.info");
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

  return app.exec();
}
