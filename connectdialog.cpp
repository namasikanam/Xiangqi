#include "connectdialog.h"
#include "ui_connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ConnectDialog) {
  ui->setupUi(this);
}

ConnectDialog::~ConnectDialog() { delete ui; }

void ConnectDialog::on_lineEdit_textChanged(const QString &arg1) {
  ui->buttonBox->setEnabled(hostAddress.setAddress(arg1));
}

bool ConnectDialog::connectToHost(QTcpSocket *socket) {
  socket->connectToHost(hostAddress,
                        static_cast<quint16>(ui->spinBox->value()));
  return socket->waitForConnected(10000);
}
