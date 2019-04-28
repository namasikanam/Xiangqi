#ifndef CD_H
#define CD_H

#include "board.h"
#include <QDialog>
#include <QHostAddress>
#include <QNetworkInterface>

namespace Ui {
class CD;
}

class CD : public QDialog {
  Q_OBJECT

public:
  explicit CD(QWidget *parent = nullptr);
  ~CD();

  void init(QString &filename, bool &offensive, quint16 &port);

private slots:

private:
  Ui::CD *ui;
};

#endif // CD_H
