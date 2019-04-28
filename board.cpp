#include "board.h"
#include "ui_board.h"

#define forBoard                                                               \
  for (int i = 0; i < 10; ++i)                                                 \
    for (int j = 0; j < 9; ++j)

Board::Board(QWidget *parent) : QWidget(parent), ui(new Ui::Board) {
  ui->setupUi(this);

  clear();
}

void Board::clear() {
  forBoard {
    piece[i][j] = noPiece;
    color[i][j] = noColor;
  }
  memset(selected, 0, sizeof(selected));
  select = 0;
  thinking = false;
}

Board::~Board() { delete ui; }

void Board::paintEvent(QPaintEvent *) {
  //  qDebug() << "Before paintEvent";
  //  forBoard qDebug("color(%d,%d)=%d\n", i, j, color[i][j]);

  QPainter p(this);

  QImage background(":/img/board/canvas.png");
  //  setGeometry(QRect(geometry().topLeft(), background.size()));
  setMinimumSize(background.size());
  setMaximumSize(background.size());
  QRect rect = QRect(QPoint(0, 0), background.size());
  p.drawImage(rect, background, rect);

  // 先找到每一个位置对应的矩形叭
  double pixelWidth = width() * 0.107, pixelHeight = height() * 0.097;
  double pieceFactor = 0.99;
  //  调试用
  //  double pieceFactor = 0.1;
  QSize pixelSize(static_cast<int>(pixelWidth), static_cast<int>(pixelHeight));
  double x0 = width() * 0.075, y0 = height() * 0.065;
  // 用于画白矩形调试
  //  p.setPen(Qt::NoPen);
  //  p.setBrush(Qt::white);
  double x, y = y0;
  for (int i = 0; i < 10; ++i) {
    x = x0;
    for (int j = 0; j < 9; ++j) {
      pieceRect[myColor == red ? i : 9 - i][j] =
          QRectF(x - pixelWidth * pieceFactor / 2,
                 y - pixelHeight * pieceFactor / 2, pixelWidth * pieceFactor,
                 pixelHeight * pieceFactor)
              .toRect();

      // 调试用
      //      p.drawRect(pieceRect[i][j]);

      x += pixelWidth;
    }
    y += pixelHeight;
  }
  //  调试用
  //  return;

  forBoard {
    QString fileName;
    if (color[i][j] == noColor)
      fileName = "w";
    else {
      static QString colorMap[] = {"r", "b"};
      static QString pieceMap[2][7] = {
          {"shuai", "shi", "xiang", "ma", "che", "pao", "bing"},
          {"jiang", "shi", "xiang", "ma", "che", "pao", "zu"}};
      fileName +=
          colorMap[color[i][j]] + '-' + pieceMap[color[i][j]][piece[i][j]];
    }
    if (selected[i][j])
      fileName += "-d";
    fileName += ".png";
    QImage pieceImage(":/img/delicate/" + fileName);
    p.drawImage(pieceRect[i][j], pieceImage,
                QRect(QPoint(0, 0), pieceImage.size()));
  }
}

void Board::setPiece(QString campaign, bool offensive) {
  qDebug("Now I will setPiece");

  clear();

  // Get Color
  Color c = static_cast<Color>(static_cast<int>([&campaign]() -> QChar {
    int i = 0;
    while (campaign[i] != 'r' && campaign[i] != 'b')
      ++i;
    return campaign[i];
  }() == QChar('b')));
  myColor = offensive ? c : static_cast<Color>(static_cast<int>(c) ^ 1);
  enemyColor = static_cast<Color>(static_cast<int>(myColor) ^ 1);

  // Get Number
  QList<QString> numbers = campaign.split(QRegularExpression("[^0-9]*"))
                               .filter(QRegularExpression("[0-9]"));
  auto read = [&numbers]() -> int {
    int x = numbers.front().toInt();
    numbers.pop_front();
    return x;
  };
  for (int i = 2; i--;) {
    for (int j = 0; j < 7; ++j) {
      int k = read();
      while (k--) {
        int x = read(), y = read();
        color[9 - y][x] = c;
        piece[9 - y][x] = static_cast<Piece>(j);
      }
    }
    c = static_cast<Color>(static_cast<int>(c) ^ 1);
  }

  qDebug("All is readed");

  update();
}

void Board::mousePressEvent(QMouseEvent *e) {
  if (thinking)
    forBoard if (pieceRect[i][j].contains(e->pos())) {
      if (select == 0 || select == 2) {
        if (color[i][j] == myColor) {
          memset(selected, 0, sizeof(selected));
          select = 1;
          selected[i][j] = 1;

          update();
        }
      } else {
        int y1, x1;
        forBoard if (selected[i][j]) {
          y1 = i;
          x1 = j;
        }
        if (accessible[y1][x1][i][j]) {
          selected[i][j] = 1;
          select = 2;

          move(y1, x1, i, j);

          emit step(y1, x1, i, j);
        } else {
          selected[y1][x1] = false;
          select = 0;

          update();
        }
      }
      break;
    }
}

void Board::move(int y1, int x1, int y2, int x2) {
  if (color[y2][x2] != noColor)
    QSound::play(":/sounds/capture.wav");

  memset(selected, 0, sizeof(selected));
  select = 2;
  selected[y2][x2] = selected[y1][x1] = 1;
  piece[y2][x2] = piece[y1][x1];
  piece[y1][x1] = noPiece;
  color[y2][x2] = color[y1][x1];
  color[y1][x1] = noColor;
  update();
}

static int dx[] = {1, 1, 0, -1, -1, -1, 0, 1, 1},
           dy[] = {0, -1, -1, -1, 0, 1, 1, 1, 0};

bool Board::jiangjun() {
  int y0, x0;
  forBoard if (color[i][j] == myColor && piece[i][j] == shuai) {
    y0 = i;
    x0 = j;
    break;
  }
  //兵, 车/将, 炮
  int y, x;
  int step, cnt;
  for (int i = 0; i < 8; i += 2) {
    y = y0 + dy[i];
    x = x0 + dx[i];
    step = 1;
    cnt = 0;
    for (; x >= 0 && x < 9 && y >= 0 && y < 10; ++step) {
      if (color[y][x] == enemyColor &&
          ((step == 1 && i != 6 && piece[y][x] == bing) ||
           (cnt == 0 && piece[y][x] == jiang) ||
           (cnt == 0 && piece[y][x] == che) ||
           (cnt == 1 && piece[y][x] == pao)))
        return 1;

      cnt += color[y][x] != noColor;
      ++step;
      y += dy[i];
      x += dx[i];
    }
  }
  //马
  static auto check = [](int dy, int dx) -> bool {
    dx = abs(dx);
    dy = abs(dy);
    return (dx == 2 && dy == 1) || (dy == 2 && dx == 1);
  };
  int dy, dx;
  forBoard if (color[i][j] == enemyColor && piece[i][j] == ma &&
               check(dy = y0 - i, dx = x0 - j) &&
               color[i + dy / 2][j + dx / 2] == noColor) return 1;
  return 0;
}

void Board::prepare() {
  qDebug() << "I will prepare.";

  if (jiangjun())
    QSound::play(":/sounds/checkmate.wav");

  memset(accessible, false, sizeof(accessible));

  // 第一遍找到所有可达之处
  int y, x;
  auto inBoard = [&y, &x]() { return y >= 0 && y < 10 && x >= 0 && x < 9; };
  auto inRect = [&y, &x](int left, int top, int width, int height) {
    return x >= left && x < left + width && y >= top && y < top + height;
  };
  auto inGong = [inRect, this] {
    return myColor == red ? inRect(3, 6, 3, 3) : inRect(3, 0, 3, 3);
  };
  auto inBan = [inRect, this] {
    return myColor == red ? inRect(0, 5, 9, 5) : inRect(0, 0, 9, 5);
  };
  forBoard if (color[i][j] == myColor) {
    switch (piece[i][j]) {
    case jiang:
      for (int k = 0; k < 8; k += 2) {
        y = i + dy[k];
        x = j + dx[k];
        if (inGong() && color[y][x] != myColor)
          accessible[i][j][y][x] = 1;
      }
      break;
    case shi:
      for (int k = 1; k < 8; k += 2) {
        y = i + dy[k];
        x = j + dx[k];
        if (inGong() && color[y][x] != myColor)
          accessible[i][j][y][x] = 1;
      }
      break;
    case xiang:
      for (int k = 1; k < 8; k += 2) {
        y = i + dy[k] * 2;
        x = j + dx[k] * 2;
        if (inBan() && color[y][x] != myColor &&
            color[i + dy[k]][j + dx[k]] == noColor)
          accessible[i][j][y][x] = 1;
      }
      break;
    case che:
      for (int k = 0; k < 8; k += 2)
        for (y = i + dy[k], x = j + dx[k]; inBoard(); y += dy[k], x += dx[k]) {
          if (color[y][x] != myColor)
            accessible[i][j][y][x] = 1;
          if (color[y][x] != noColor)
            break;
        }
      break;
    case ma:
      //      qDebug("I'm checking (%d,%d)", i, j);
      for (int k = 0; k < 8; ++k) {
        y = i + dy[k] + dy[k + 1];
        x = j + dx[k] + dx[k + 1];
        int tk = k + (k & 1);

        //        qDebug("Destination:(%d,%d) leg:(%d,%d)", y, x, i + dy[tk], j
        //        + dx[tk]);

        if (inBoard() && color[y][x] != myColor &&
            color[i + dy[tk]][j + dx[tk]] == noColor)
          accessible[i][j][y][x] = 1;
      }
      break;
    case pao:
      for (int k = 0; k < 8; k += 2) {
        y = i + dy[k];
        x = j + dx[k];
        for (bool bridge = false; inBoard(); y += dy[k], x += dx[k])
          if (color[y][x] == noColor)
            accessible[i][j][y][x] = !bridge;
          else {
            if (bridge) {
              accessible[i][j][y][x] = color[y][x] == enemyColor;
              break;
            } else
              bridge = true;
          }
      }
      break;
    case zu:
      if (myColor == red) {
        y = i - 1;
        x = j;
      } else {
        y = i + 1;
        x = j;
      }
      if (inBoard() && color[y][x] != myColor)
        accessible[i][j][y][x] = 1;
      if (!inBan())
        for (int k = 0; k < 8; k += 4) {
          y = i + dy[k];
          x = j + dx[k];
          if (inBoard() && color[y][x] != myColor)
            accessible[i][j][y][x] = 1;
        }
      break;
    case noPiece:
      break;
    }
  }

  qDebug() << "I've found all accessible!";

  // 第二遍判断到了可达之处后是否会被将军
  Color tc;
  Piece tp;
  forBoard for (int k = 0; k < 10; ++k) for (int o = 0; o < 9;
                                             ++o) if (accessible[i][j][k][o]) {
    tc = color[k][o];
    tp = piece[k][o];

    color[k][o] = color[i][j];
    piece[k][o] = piece[i][j];
    color[i][j] = noColor;
    piece[i][j] = noPiece;

    accessible[i][j][k][o] &= !jiangjun();

    color[i][j] = color[k][o];
    piece[i][j] = piece[k][o];
    color[k][o] = tc;
    piece[k][o] = tp;
  }

  qDebug("Can I move now?");

  // 是否被将死
  forBoard for (int k = 0; k < 10;
                ++k) for (int o = 0; o < 9;
                          ++o) if (accessible[i][j][k][o]) return;

  qDebug("No, I'm dead.");

  emit surrender();
}

void Board::output(QTextStream &ts, Color c) {
  qDebug() << "output\n";

  if (c == red)
    ts << "red\n";
  else
    ts << "black\n";
  for (int k = 0; k < 7; ++k) {
    int count = 0;
    forBoard count += color[i][j] == c && static_cast<int>(piece[i][j]) == k;
    ts << count;
    forBoard if (color[i][j] == c && static_cast<int>(piece[i][j]) == k) ts
        << " <" << j << 9 - i << ">";
    ts << endl;
  }
}

void Board::save() {
  qDebug() << "save\n";

  QFile file("../assignment/campaign/" +
             QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss") +
             ".txt");
  file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
  QTextStream ts(&file);
  output(ts, static_cast<Color>(myColor ^ thinking ^ 1));
  output(ts, static_cast<Color>(myColor ^ thinking));
}
