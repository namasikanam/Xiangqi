#ifndef ACTION_H
#define ACTION_H

#include <QtWidgets>

class MyAction : public QWidgetAction {
  Q_OBJECT

public:
  explicit MyAction(QObject *parent = nullptr);
  ~MyAction() override;

  QWidget *createWidget(QWidget *parent) override;
};

#endif // ACTION_H
