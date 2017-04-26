#include <QApplication>
#include <QProcess>
#include "Nuclei.h"
#include "ENSDFDataSource.h"

int main(int argc, char *argv[])
{
//  qRegisterMetaType<Decay::CascadeIdentifier>("CascadeIdentifier");
  //    qRegisterMetaTypeStreamOperators<Decay::CascadeIdentifier>("CascadeIdentifier");

  qRegisterMetaType<ENSDFTreeItem>("ENSDFTreeItem");
  qRegisterMetaTypeStreamOperators<ENSDFTreeItem>("ENSDFTreeItem");

  QApplication a(argc, argv);
  QCoreApplication::setOrganizationName(QString::fromUtf8("Uni-Göttingen"));
  QCoreApplication::setOrganizationDomain("physik.uni-goettingen.de");
  QCoreApplication::setApplicationName("Nuclei");

  int retcode = 0;
  {
    Nuclei w;
    w.show();
    
    retcode = a.exec();
  }

  if (retcode == 6000) {
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    return 0;
  }

  return retcode;
}
