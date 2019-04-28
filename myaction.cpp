#include "myaction.h"

MyAction::MyAction(QObject *parent) : QWidgetAction(parent) {}

QWidget *MyAction::createWidget(QWidget *parent) {
  //  qDebug() << "This is your text:" << this->text();

  QToolButton *tb = new QToolButton(parent);
  tb->setDefaultAction(this);
  tb->setText(this->text()); // override text stripping
  tb->setFocusPolicy(Qt::NoFocus);

  return tb;
}

MyAction::~MyAction() {}
