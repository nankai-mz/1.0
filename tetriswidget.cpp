#include "tetriswidget.h"
#include <cstring>  // 用于 memcpy

const QVector<QRgb> TetrisWidget::colors = {
    0x0000FF, 0xFF0000, 0x00FF00, 0xFF00FF,
    0xFFFF00, 0x00FFFF, 0xFFA500
};

// 构造函数
TetrisWidget::TetrisWidget(QWidget *parent) : QWidget(parent),
    currentX(0), currentY(0), score(0) {
    setFixedSize(400, 600);
    clearBoard();
    newPiece();
    timer.start(500, this);
}

// 绘图事件
void TetrisWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    drawBoard(p);
    drawPiece(p, currentPiece, currentX, currentY);
}

// 键盘事件
void TetrisWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Left:
        tryMove(currentPiece, currentX - 1, currentY);
        break;
    case Qt::Key_Right:
        tryMove(currentPiece, currentX + 1, currentY);
        break;
    case Qt::Key_Down:
        tryMove(currentPiece.rotatedRight(), currentX, currentY);
        break;
    case Qt::Key_Up:
        tryMove(currentPiece.rotatedLeft(), currentX, currentY);
        break;
    case Qt::Key_Space:
        dropDown();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

// 定时器事件
void TetrisWidget::timerEvent(QTimerEvent *event) {
    if (event->timerId() == timer.timerId()) {
        oneLineDown();
    } else {
        QWidget::timerEvent(event);
    }
}

// 清空棋盘
void TetrisWidget::clearBoard() {
    for (int i = 0; i < BoardHeight; ++i)
        for (int j = 0; j < BoardWidth; ++j)
            board[i][j] = 0;
}

// 方块旋转方法
TetrisWidget::Piece TetrisWidget::Piece::rotatedRight() const {
    Piece result;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result.shape[i][j] = shape[3 - j][i];
    result.color = color;
    return result;
}

TetrisWidget::Piece TetrisWidget::Piece::rotatedLeft() const {
    Piece result;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result.shape[i][j] = shape[j][3 - i];
    result.color = color;
    return result;
}

// 生成新方块
void TetrisWidget::newPiece() {
    static const int shapes[7][4][4] = {
        {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}}, // I
        {{1,1,1,0}, {1,0,0,0}, {0,0,0,0}, {0,0,0,0}}, // L
        {{1,1,1,0}, {0,0,1,0}, {0,0,0,0}, {0,0,0,0}}, // J
        {{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, // O
        {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, // S
        {{1,1,1,0}, {0,1,0,0}, {0,0,0,0}, {0,0,0,0}}, // T
        {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}  // Z
    };

    int index = QRandomGenerator::global()->bounded(7);
    currentPiece.color = colors[index];
    memcpy(currentPiece.shape, shapes[index], sizeof(currentPiece.shape));
    
    currentX = BoardWidth / 2 - 2;
    currentY = 0;

    if (!tryMove(currentPiece, currentX, currentY)) {
        timer.stop();
        QMessageBox::information(this, "Game Over", "Game Over!");
    }
}

// 移动检测
bool TetrisWidget::tryMove(const Piece &newPiece, int newX, int newY) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (newPiece.shape[i][j]) {
                int x = newX + j;
                int y = newY + i;
                if (x < 0 || x >= BoardWidth || y >= BoardHeight)
                    return false;
                if (y >= 0 && board[y][x])
                    return false;
            }
        }
    }
    currentPiece = newPiece;
    currentX = newX;
    currentY = newY;
    update();
    return true;
}

// 快速下落
void TetrisWidget::dropDown() {
    while (oneLineDown());
}

// 单步下落
bool TetrisWidget::oneLineDown() {
    if (!tryMove(currentPiece, currentX, currentY + 1)) {
        pieceDropped();
        return false;
    }
    return true;
}

// 方块落地处理
void TetrisWidget::pieceDropped() {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (currentPiece.shape[i][j]) {
                int x = currentX + j;
                int y = currentY + i;
                if (y >= 0) board[y][x] = currentPiece.color;
            }
        }
    }
    removeFullLines();
    newPiece();
}

// 消除满行
void TetrisWidget::removeFullLines() {
    int numFullLines = 0;
    for (int y = BoardHeight - 1; y >= 0; --y) {
        bool full = true;
        for (int x = 0; x < BoardWidth; ++x) {
            if (!board[y][x]) {
                full = false;
                break;
            }
        }
        if (full) {
            ++numFullLines;
            for (int yy = y; yy > 0; --yy)
                memcpy(board[yy], board[yy-1], BoardWidth * sizeof(int));
            ++y;
        }
    }
    if (numFullLines > 0) {
        score += 100 * numFullLines;
        emit scoreChanged(score);
        update();
    }
}

// 绘制棋盘
void TetrisWidget::drawBoard(QPainter &p) {
    for (int y = 0; y < BoardHeight; ++y) {
        for (int x = 0; x < BoardWidth; ++x) {
            if (board[y][x]) {
                p.fillRect(x * 30, y * 30, 29, 29, QColor(board[y][x]));
            }
        }
    }
}

// 绘制当前方块
void TetrisWidget::drawPiece(QPainter &p, const Piece &piece, int x, int y) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (piece.shape[i][j]) {
                p.fillRect((x + j) * 30, (y + i) * 30, 29, 29, QColor(piece.color));
            }
        }
    }
}
