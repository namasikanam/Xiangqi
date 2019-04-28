#ifndef BOARD_H
#define BOARD_H

#include <QMediaPlayer>
#include <QSound>
#include <QtWidgets>

namespace Ui {
class Board;
}

enum Piece { jiang, shi, xiang, ma, che, pao, bing, noPiece };
#define shuai jiang
#define zu bing
enum Color { red, black, noColor };

class Board : public QWidget {
  Q_OBJECT

public:
  explicit Board(QWidget *parent = nullptr);
  ~Board();

  void clear();
  void setPiece(QString campaign, bool offensive);

  void move(int y1, int x1, int y2, int x2);
  void prepare(); // 轮到自己走了, 看一看那里能走, 有没有被将军, 有没有被将死
  bool jiangjun(); // 有没有被将军呀

  bool thinking = false;
  Color myColor, enemyColor;

signals:
  void surrender();
  void step(int y1, int x1, int y2, int x2);

public slots:
  void save();

private:
  void paintEvent(QPaintEvent *);
  void mousePressEvent(QMouseEvent *e);
  Ui::Board *ui;

  Piece piece[10][10];
  Color color[10][10];
  QRect pieceRect[10][10];
  bool selected[10][10];
  int select;
  bool accessible[10][10][10][10];

  void output(QTextStream &ts, Color color);
};

#endif // BOARD_H
