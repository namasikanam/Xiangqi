#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "board.h"
#include "cd.h"
#include "connectdialog.h"
#include "myaction.h"
#include "waitdialog.h"
#include <QJsonDocument>
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void startGame();
  void endGame(bool victory);
  void startThink();
  void endThink();

public slots:
  void read();
  void step(int y1, int x1, int y2, int x2);
  void surrender();

private:
  Ui::MainWindow *ui;

  // 棋盘
  Board *board = new Board(this);
  bool offensive;
  bool thinking = false;

  // action
  MyAction *actionCreate, *actionConnect, *actionSave, *actionSurrender;
  void initCreate();
  void initConnect();
  void initSave();
  void initSurrender();

  // 网络
  QTcpServer *server = new QTcpServer;
  QTcpSocket *socket;

  // 计时器
  QTimer *timer = new QTimer(this);
  int restTime;
};

#endif // MAINWINDOW_H
