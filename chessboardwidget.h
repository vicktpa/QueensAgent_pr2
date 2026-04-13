#ifndef CHESSBOARDWIDGET_H
#define CHESSBOARDWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <vector>
#include "KnowledgeBase.h"

/**
 * Виджет для отображения шахматной доски и ферзей
 *
 * Отвечает за визуализацию шахматной доски, отображение ферзей,
 * наложение визуализации базы знаний (веса и цвета клеток),
 * а также обработку событий мыши для редактирования базы знаний.
 */
class ChessBoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChessBoardWidget(QWidget* parent = nullptr);

    // Управление размером доски
    void setBoardSize(int size);
    int getBoardSize() const { return m_boardSize; }

    // Управление позициями ферзей
    void setQueenPositions(const std::vector<int>& positions);
    void clearQueens();

    // Управление решениями
    const std::vector<std::vector<int>>& getAllSolutions() const { return m_allSolutions; }
    int getCurrentSolutionIndex() const { return m_currentSolutionIndex; }
    void setCurrentSolutionIndex(int index) { m_currentSolutionIndex = index; }

    // Управление базой знаний
    void setKnowledgeBase(KnowledgeBase* kb) { m_knowledgeBase = kb; }
    void setEditMode(bool enabled) { m_editMode = enabled; }
    bool isEditMode() const { return m_editMode; }

    // Обновление отображения
    void refreshDisplay();

public slots:
    // Навигация по решениям
    void nextSolution();
    void previousSolution();
    void setSolutions(const std::vector<std::vector<int>>& solutions);

protected:
    // Переопределенные методы Qt
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

signals:
    // Сигналы для взаимодействия с базой знаний
    void cellClicked(int col, int row);
    void cellWeightChanged(int col, int row, int newWeight);
    void cellColorChanged(int col, int row, PositionColor newColor);

private:
    // Методы отрисовки
    void drawBoard(QPainter& painter);           // Отрисовка шахматной доски
    void drawQueens(QPainter& painter);          // Отрисовка ферзей
    void drawKnowledgeOverlay(QPainter& painter); // Отрисовка данных из БЗ
    QRect getCellRect(int col, int row) const;    // Получение прямоугольника клетки
    void drawQueenAt(QPainter& painter, int col, int row); // Отрисовка одного ферзя

    // Вспомогательные методы
    QPair<int, int> getCellAtPosition(const QPoint& pos) const; // Определение клетки по координатам
    void showEditCellDialog(int col, int row);    // Диалог редактирования клетки

private:
    // Данные о доске и решениях
    int m_boardSize;                    // Размер доски (N x N)
    int m_cellSize;                     // Размер одной клетки в пикселях
    std::vector<int> m_queenPositions;  // Позиции ферзей (индекс - столбец, значение - строка)
    std::vector<std::vector<int>> m_allSolutions; // Все найденные решения
    int m_currentSolutionIndex;         // Индекс текущего отображаемого решения

    // База знаний и режимы
    KnowledgeBase* m_knowledgeBase;     // Указатель на базу знаний
    bool m_editMode;                    // Режим редактирования базы знаний
};

#endif // CHESSBOARDWIDGET_H