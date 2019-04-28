#include "cd.h"
#include "ui_cd.h"

CD::CD(QWidget *parent) : QDialog(parent), ui(new Ui::CD) {
  ui->setupUi(this);

  QList<QString> ips;
  foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    if (address != QHostAddress(QHostAddress::LocalHost) &&
        address != QHostAddress(QHostAddress::LocalHostIPv6))
      ips.append(address.toString());
  ui->comboBox->addItems(ips);
  ui->comboBox->setCurrentIndex(0);

  QStringList campaign = QDir("../assignment/campaign")
                             .entryList(QDir::Files | QDir::Readable)
                             .filter(QRegularExpression("[.]txt$"));
  for (int i = campaign.size(); i--;)
    if (campaign[i] == "default.txt")
      swap(campaign[0], campaign[i]);
  for (int i = campaign.size(); i--;)
    campaign[i].remove(campaign[i].size() - 4, 4);
  ui->listWidget->addItems(campaign);
  ui->listWidget->setCurrentRow(0);
}

CD::~CD() { delete ui; }

void CD::init(QString &filename, bool &offensive, quint16 &port) {
  filename = ui->listWidget->currentItem()->text();
  offensive = ui->buttonOffensive->isChecked();
  port = static_cast<quint16>(ui->spinBox->value());
}
