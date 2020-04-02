#include <QApplication>
#include <QProcess>
#include "Nuclei.h"
#include "ENSDFDataSource.h"

#include <QCommandLineParser>

int main(int argc, char *argv[])
{
//  qRegisterMetaType<Decay::CascadeIdentifier>("CascadeIdentifier");
  //    qRegisterMetaTypeStreamOperators<Decay::CascadeIdentifier>("CascadeIdentifier");

  qRegisterMetaType<ENSDFTreeItem>("ENSDFTreeItem");
  qRegisterMetaTypeStreamOperators<ENSDFTreeItem>("ENSDFTreeItem");

  QApplication application(argc, argv);
  QCoreApplication::setOrganizationName(QString::fromUtf8("ARL"));
  QCoreApplication::setApplicationName("Nuclei");

  /// Parse command line
  QCommandLineParser parser;
  parser.setApplicationDescription("Interactive ENSDF viewer");
  parser.addHelpOption();
  //  parser.addVersionOption();
  QCommandLineOption dieOption(QStringList() << "d" << "die",
                               QApplication::translate("main", "Die right away (for testing)"));
  parser.addOption(dieOption);
  parser.process(application);
  bool die_now = parser.isSet(dieOption);

  if (parser.isSet("h") || die_now)
    return EXIT_SUCCESS;

  int retcode = 0;
  {
    Nuclei w;
    w.show();
    
    retcode = application.exec();
  }

  if (retcode == 6000)
  {
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    return 0;
  }

  return retcode;
}
