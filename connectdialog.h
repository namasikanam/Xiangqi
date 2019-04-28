#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include <QHostAddress>
#include <QTcpSocket>

namespace Ui {
class ConnectDialog;
}

class ConnectDialog : public QDialog {
  Q_OBJECT

public:
  explicit ConnectDialog(QWidget *parent = nullptr);
  ~ConnectDialog();

  bool connectToHost(QTcpSocket *socket);

private slots:
  void on_lineEdit_textChanged(const QString &arg1);

private:
  Ui::ConnectDialog *ui;

  QHostAddress hostAddress = QHostAddress::LocalHost;
};

#endif // CONNECTDIALOG_H
