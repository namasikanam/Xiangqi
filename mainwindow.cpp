#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  setCentralWidget(board);
  setMaximumSize(size());

  initCreate();
  initConnect();
  initSave();
  initSurrender();

  connect(board, &Board::surrender, this, &MainWindow::surrender);
  connect(board, &Board::step, this, &MainWindow::step);

  connect(timer, &QTimer::timeout, [this]() {
    if (restTime == 0)
      surrender();
    else
      this->statusBar()->showMessage(
          "Rest time: " + QString::number(restTime--) + "s");
  });
}

MainWindow::~MainWindow() {
  delete ui;
  delete server;
}

void MainWindow::read() {
  qDebug("Something I could readed is sended to me.");

  QJsonDocument json = QJsonDocument::fromJson(socket->readAll());

  qDebug() << "head: " << json["head"];

  if (json["head"] == "campaign") {
    offensive = json["offensive"].toBool();
    board->setPiece(json["campaign"].toString(), offensive);

    qDebug() << "read offensive:" << offensive;
  } else if (json["head"] == "step") {
    board->move(json["y1"].toInt(), json["x1"].toInt(), json["y2"].toInt(),
                json["x2"].toInt());
    startThink();
  } else if (json["head"] == "surrender") {
    endGame(1);
  }
}

void MainWindow::startThink() {
  qDebug("Now Let's start to think.");

  thinking = board->thinking = true;

  // 计时器启动!
  restTime = 20;
  timer->start(1000);

  board->prepare();
}

void MainWindow::endThink() {
  qDebug("endThink[");

  // 计时器停止
  timer->stop();
  statusBar()->clearMessage();

  thinking = board->thinking = false;

  qDebug("endThink]");
}

void MainWindow::step(int y1, int x1, int y2, int x2) {
  endThink();

  QJsonObject json;
  json.insert("head", "step");
  json.insert("y1", y1);
  json.insert("x1", x1);
  json.insert("y2", y2);
  json.insert("x2", x2);
  socket->write(QJsonDocument(json).toJson());
  socket->waitForBytesWritten();
}

void MainWindow::surrender() {
  QJsonObject json;
  json.insert("head", "surrender");
  socket->write(QJsonDocument(json).toJson());
  socket->waitForBytesWritten();

  endGame(0);
}

void MainWindow::startGame() {
  // disable some actions, enable some actions
  actionCreate->setEnabled(false);
  actionConnect->setEnabled(false);
  actionSave->setEnabled(true);
  actionSurrender->setEnabled(true);

  //  ui->statusBar->showMessage(QString("You are ") +
  //                             (board->myColor == red ? "red" : "black") +
  //                             ".");

  qDebug() << "startGame offensive:" << offensive;

  if (offensive)
    startThink();
}

void MainWindow::endGame(bool victory) {
  qDebug("endGame[");

  endThink();

  // 需要MessageBox, 点击确定之后清空一切.
  QMessageBox msgBox;
  msgBox.setText(victory ? "Congratulations to you!"
                         : "Good game, well played!");
  msgBox.exec();

  // 清空一切
  board->clear();
  board->update();

  socket->close();
  delete socket;

  actionCreate->setEnabled(true);
  actionConnect->setEnabled(true);
  actionSave->setEnabled(false);
  actionSurrender->setEnabled(false);

  qDebug("endGame]");
}

void MainWindow::initCreate() {
  actionCreate = new MyAction(this);
  actionCreate->setText("&Create");
  ui->mainToolBar->addAction(actionCreate);
  connect(actionCreate, &MyAction::triggered, [this]() {
    CD createDialog(this);
    if (createDialog.exec()) {
      // 把createDialog那里的数据搞过来
      QString filename;
      quint16 port;
      createDialog.init(filename, offensive, port);

      // 搞一个QTcpServer
      server->listen(QHostAddress::Any, port);

      // 等人来
      WaitDialog waitDialog(this);
      connect(server, &QTcpServer::newConnection, &waitDialog,
              &WaitDialog::accept);

      if (waitDialog.exec()) {
        // 人来了
        socket = server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &MainWindow::read);

        qDebug("Success: Get you finally!");

        // 给client把campaign发过去.
        QJsonObject json;
        json.insert("head", "campaign");
        json.insert("offensive", !offensive);
        QFile file("../assignment/campaign/" + filename + ".txt");
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString campaign = QTextStream(&file).readAll();
        json.insert("campaign", campaign);
        socket->write(QJsonDocument(json).toJson());
        board->setPiece(campaign, offensive);
        socket->waitForBytesWritten();

        // 开始游戏
        startGame();
      } else {
        // 人没来
        qDebug("Waiting Cancelled.");
      }

      server->close();
    }
  });
}

void MainWindow::initConnect() {
  actionConnect = new MyAction(this);
  actionConnect->setText("C&onnect");
  ui->mainToolBar->addAction(actionConnect);
  connect(actionConnect, &MyAction::triggered, [this]() {
    ConnectDialog connectDialog(this);
    if (connectDialog.exec()) {
      //  这里new一个, endGame的时候记得回收!
      socket = new QTcpSocket;
      if (connectDialog.connectToHost(socket)) {
        // 连上房主啦QAQ
        qDebug("Success: Server found!");
        connect(socket, &QTcpSocket::readyRead, this, &MainWindow::read);

        // 接收campaign
        // 突然发现接收其实不必写在这里...
        socket->waitForReadyRead();

        qDebug("I should have read something.");

        startGame();
      } else {
        qDebug("Error: Connect Failed");
        delete socket;
      }
    }
  });
}

void MainWindow::initSave() {
  actionSave = new MyAction(this);
  actionSave->setText("&Save");
  actionSave->setEnabled(false);
  connect(actionSave, &MyAction::triggered, board, &Board::save);
  ui->mainToolBar->addAction(actionSave);
}

void MainWindow::initSurrender() {
  actionSurrender = new MyAction(this);
  actionSurrender->setText("S&urrender");
  actionSurrender->setEnabled(false);
  connect(actionSurrender, &MyAction::triggered, this, &MainWindow::surrender);
  ui->mainToolBar->addAction(actionSurrender);
}
