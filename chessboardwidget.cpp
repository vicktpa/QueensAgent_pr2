#include "ChessBoardWidget.h"
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <cmath>

// ============================================================================
// Конструктор и основные методы
// ============================================================================

ChessBoardWidget::ChessBoardWidget(QWidget* parent)
    : QWidget(parent)
    , m_boardSize(8)           // Размер доски по умолчанию 8x8
    , m_cellSize(60)           // Начальный размер клетки 60 пикселей
    , m_currentSolutionIndex(0) // Начинаем с первого решения
    , m_knowledgeBase(nullptr)  // База знаний пока не подключена
    , m_editMode(false)         // Режим редактирования выключен
{
    // Устанавливаем минимальный размер виджета для корректного отображения
    setMinimumSize(400, 400);
    // Политика расширения - виджет может растягиваться
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Включаем отслеживание движения мыши для изменения курсора
    setMouseTracking(true);
}

/**
 *  Устанавливает новый размер доски
 *  size Новый размер (N для доски N x N)
 *
 * При изменении размера доски сбрасываются все позиции ферзей,
 * так как старые позиции могут быть некорректными для нового размера.
 */
void ChessBoardWidget::setBoardSize(int size){
    m_boardSize = size;
    // Инициализируем вектор позиций ферзей значением -1 (нет ферзя)
    m_queenPositions.assign(m_boardSize, -1);
    // Запрашиваем перерисовку
    update();
}

/**
 *  Устанавливает позиции ферзей из вектора
 *  positions Вектор, где индекс - столбец, значение - строка ферзя
 *
 * Если переданный вектор короче размера доски, недостающие столбцы
 * заполняются значением -1 (нет ферзя).
 */
void ChessBoardWidget::setQueenPositions(const std::vector<int>& positions){
    if (positions.size() >= static_cast<size_t>(m_boardSize)) {
        // Если позиций достаточно, копируем их
        m_queenPositions = positions;
        // Обрезаем до нужного размера (на случай, если вектор длиннее)
        m_queenPositions.resize(m_boardSize, -1);
    } else {
        // Если позиций меньше, заполняем -1 и копируем существующие
        m_queenPositions.assign(m_boardSize, -1);
        for (size_t i = 0; i < positions.size(); ++i)
            if (i < static_cast<size_t>(m_boardSize))
                m_queenPositions[i] = positions[i];
    }
    // Запрашиваем перерисовку
    update();
}

/**
 *  Очищает все ферзи с доски
 */
void ChessBoardWidget::clearQueens(){
    m_queenPositions.assign(m_boardSize, -1);
    update();
}

/**
 *  Переход к следующему решению (по кольцу)
 */
void ChessBoardWidget::nextSolution(){
    if (m_allSolutions.empty()) return;
    // Инкрементируем индекс по модулю количества решений
    m_currentSolutionIndex = (m_currentSolutionIndex + 1) % m_allSolutions.size();
    setQueenPositions(m_allSolutions[m_currentSolutionIndex]);
}

/**
 *  Переход к предыдущему решению (по кольцу)
 */
void ChessBoardWidget::previousSolution(){
    if (m_allSolutions.empty()) return;
    // Декрементируем индекс с учетом модуля
    m_currentSolutionIndex = (m_currentSolutionIndex - 1 + m_allSolutions.size()) % m_allSolutions.size();
    setQueenPositions(m_allSolutions[m_currentSolutionIndex]);
}

/**
 *  Устанавливает все найденные решения
 *  solutions Вектор всех решений (каждое решение - вектор позиций ферзей)
 *
 * Сохраняет все решения и отображает первое из них.
 */
void ChessBoardWidget::setSolutions(const std::vector<std::vector<int>>& solutions){
    m_allSolutions = solutions;
    m_currentSolutionIndex = 0;
    if (!solutions.empty())
        setQueenPositions(solutions[0]);
    update();
}

/**
 *  Обновляет отображение с пересчетом размера клеток
 *
 * Вызывается при изменении размера окна или принудительном обновлении.
 */
void ChessBoardWidget::refreshDisplay(){
    // Пересчитываем размер клеток при изменении размера виджета
    int widgetSize = qMin(width(), height());
    if (widgetSize > 0) {
        m_cellSize = widgetSize / m_boardSize;
    }
    update();  // Вызываем перерисовку
}

// ============================================================================
// Обработка событий мыши (для режима редактирования)
// ============================================================================

/**
 *  Обработчик нажатия кнопки мыши
 *
 * В режиме редактирования при клике на клетку открывает диалог
 * для изменения веса и цвета этой клетки в базе знаний.
 */
void ChessBoardWidget::mousePressEvent(QMouseEvent* event){
    if (event->button() == Qt::LeftButton && m_editMode) {
        QPair<int, int> cell = getCellAtPosition(event->pos());
        int col = cell.first;
        int row = cell.second;

        if (col >= 0 && col < m_boardSize && row >= 0 && row < m_boardSize) {
            showEditCellDialog(col, row);
        }
    }
    QWidget::mousePressEvent(event);
}

/**
 *  Обработчик движения мыши
 *
 * В режиме редактирования меняет курсор на "указывающий"
 * при наведении на любую клетку доски.
 */
void ChessBoardWidget::mouseMoveEvent(QMouseEvent* event){
    if (m_editMode) {
        QPair<int, int> cell = getCellAtPosition(event->pos());
        if (cell.first >= 0) {
            setCursor(Qt::PointingHandCursor);  // Рука при наведении на клетку
        } else {
            setCursor(Qt::ArrowCursor);         // Обычная стрелка вне доски
        }
    }
    QWidget::mouseMoveEvent(event);
}

/**
 *  Определяет клетку по координатам мыши
 *  pos Позиция курсора в координатах виджета
 *  Результат: пара (столбец, строка) или (-1, -1) если вне доски
 */
QPair<int, int> ChessBoardWidget::getCellAtPosition(const QPoint& pos) const{
    int col = pos.x() / m_cellSize;
    int row = pos.y() / m_cellSize;

    if (col >= 0 && col < m_boardSize && row >= 0 && row < m_boardSize) {
        return qMakePair(col, row);
    }
    return qMakePair(-1, -1);
}

/**
 *  Показывает диалог редактирования клетки
 *  col Столбец клетки (0-индексация)
 *  row Строка клетки (0-индексация)
 *
 * Диалог позволяет изменить вес (1-100) и цвет (красный/зеленый/синий)
 * выбранной клетки в базе знаний.
 */
void ChessBoardWidget::showEditCellDialog(int col, int row){
    // Проверяем, что база знаний инициализирована
    if (!m_knowledgeBase) {
        QMessageBox::warning(this, "Ошибка", "База знаний не инициализирована");
        return;
    }

    // В базе знаний строки нумеруются с 1, поэтому преобразуем
    int actualRow = row + 1;
    int currentWeight = m_knowledgeBase->getPositionWeight(col, actualRow);
    PositionColor currentColor = m_knowledgeBase->getPositionColor(col, actualRow);

    // Создаем диалоговое окно
    QDialog dialog(this);
    dialog.setWindowTitle(QString("Редактирование клетки %1%2")
                              .arg(QChar('A' + col))
                              .arg(row + 1));
    dialog.setModal(true);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Информационная метка
    QLabel* infoLabel = new QLabel(QString("Клетка: %1%2")
                                       .arg(QChar('A' + col))
                                       .arg(row + 1), &dialog);
    layout->addWidget(infoLabel);

    // Поле для ввода веса
    QHBoxLayout* weightLayout = new QHBoxLayout();
    QLabel* weightLabel = new QLabel("Вес (1-100):", &dialog);
    QSpinBox* weightSpinBox = new QSpinBox(&dialog);
    weightSpinBox->setRange(1, 100);
    weightSpinBox->setValue(currentWeight);
    weightLayout->addWidget(weightLabel);
    weightLayout->addWidget(weightSpinBox);
    layout->addLayout(weightLayout);

    // Выбор цвета
    QHBoxLayout* colorLayout = new QHBoxLayout();
    QLabel* colorLabel = new QLabel("Цвет метки:", &dialog);
    QComboBox* colorCombo = new QComboBox(&dialog);
    colorCombo->addItem("Красный", QVariant::fromValue(PositionColor::Red));
    colorCombo->addItem("Зеленый", QVariant::fromValue(PositionColor::Green));
    colorCombo->addItem("Синий", QVariant::fromValue(PositionColor::Blue));

    // Устанавливаем текущий цвет
    int currentIndex = 0;
    switch (currentColor) {
    case PositionColor::Red: currentIndex = 0; break;
    case PositionColor::Green: currentIndex = 1; break;
    case PositionColor::Blue: currentIndex = 2; break;
    }
    colorCombo->setCurrentIndex(currentIndex);

    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(colorCombo);
    layout->addLayout(colorLayout);

    // Область предпросмотра цвета
    QLabel* previewLabel = new QLabel("Предпросмотр:", &dialog);
    QWidget* previewWidget = new QWidget(&dialog);
    previewWidget->setFixedSize(50, 50);

    // Лямбда-функция для обновления предпросмотра
    auto updatePreview = [previewWidget](PositionColor color) {
        QColor displayColor;
        switch (color) {
        case PositionColor::Red: displayColor = QColor(255, 100, 100); break;
        case PositionColor::Green: displayColor = QColor(100, 255, 100); break;
        case PositionColor::Blue: displayColor = QColor(100, 100, 255); break;
        default: displayColor = QColor(200, 200, 200); break;
        }
        previewWidget->setStyleSheet(QString("background-color: rgb(%1, %2, %3); border: 1px solid black;")
                                         .arg(displayColor.red())
                                         .arg(displayColor.green())
                                         .arg(displayColor.blue()));
    };

    updatePreview(currentColor);

    QHBoxLayout* previewLayout = new QHBoxLayout();
    previewLayout->addWidget(previewLabel);
    previewLayout->addWidget(previewWidget);
    previewLayout->addStretch();
    layout->addLayout(previewLayout);

    // Обновляем предпросмотр при изменении выбора цвета
    connect(colorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [updatePreview, colorCombo](int index) {
                Q_UNUSED(index)
                PositionColor color = colorCombo->currentData().value<PositionColor>();
                updatePreview(color);
            });

    // Кнопки OK/Отмена
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("Применить", &dialog);
    QPushButton* cancelButton = new QPushButton("Отмена", &dialog);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    // Обработчик кнопки Применить
    connect(okButton, &QPushButton::clicked, [&]() {
        int newWeight = weightSpinBox->value();
        PositionColor newColor = colorCombo->currentData().value<PositionColor>();

        // Сохраняем изменения в базу знаний
        m_knowledgeBase->setPositionWeight(col, actualRow, newWeight);
        m_knowledgeBase->setPositionColor(col, actualRow, newColor);

        // Сигнализируем об изменениях
        emit cellWeightChanged(col, actualRow, newWeight);
        emit cellColorChanged(col, actualRow, newColor);

        dialog.accept();
        update();  // Перерисовываем доску
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

// ============================================================================
// Отрисовка виджета
// ============================================================================

/**
 *  Обработчик события перерисовки
 *
 * Выполняет полную перерисовку доски в следующем порядке:
 * 1. Шахматная доска
 * 2. Наложение данных из базы знаний (веса и цвета)
 * 3. Ферзи
 */
void ChessBoardWidget::paintEvent(QPaintEvent* event){
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // Включаем сглаживание

    drawBoard(painter);                 // Рисуем шахматную доску

    if (m_knowledgeBase) {
        drawKnowledgeOverlay(painter);  // Рисуем наложение БЗ (есть база знаний)
    }

    drawQueens(painter);                // Рисуем ферзей поверх всего
}

/**
 *  Обработчик изменения размера окна
 *
 * Пересчитывает размер клетки и вызывает перерисовку.
 */
void ChessBoardWidget::resizeEvent(QResizeEvent* event){
    Q_UNUSED(event)
    int widgetSize = qMin(width(), height());
    if (widgetSize > 0 && m_boardSize > 0) {
        m_cellSize = widgetSize / m_boardSize;
    }
    update();
}

/**
 *  Рисует шахматную доску
 *  painter Объект QPainter для рисования
 *
 * Клетки чередуются: светлые (240,217,181) и темные (181,136,99).
 */
void ChessBoardWidget::drawBoard(QPainter& painter){
    for (int i = 0; i < m_boardSize; ++i) {
        for (int j = 0; j < m_boardSize; ++j) {
            QRect rect = getCellRect(i, j);
            // Чередуем цвета клеток
            if ((i + j) % 2 == 0)
                painter.fillRect(rect, QColor(240, 217, 181));  // Светлая клетка
            else
                painter.fillRect(rect, QColor(181, 136, 99));   // Темная клетка
            // Рисуем границу клетки
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(rect);
        }
    }
}

/**
 *  Рисует наложение данных из базы знаний
 *  painter Объект QPainter для рисования
 *
 * Для каждой клетки:
 * - Заливает полупрозрачным цветом в зависимости от цвета в БЗ
 * - Отображает вес клетки в верхней половине клетки
 */
void ChessBoardWidget::drawKnowledgeOverlay(QPainter& painter){
    if (!m_knowledgeBase) return;

    painter.setPen(Qt::black);

    for (int col = 0; col < m_boardSize; ++col) {
        for (int row = 0; row < m_boardSize; ++row) {
            QRect rect = getCellRect(col, row);
            int actualRow = row + 1;  // В БЗ строки с 1

            // Получаем данные из БЗ
            int weight = m_knowledgeBase->getPositionWeight(col, actualRow);
            PositionColor color = m_knowledgeBase->getPositionColor(col, actualRow);

            // Рисуем полупрозрачный цветовой фон
            QColor overlayColor = m_knowledgeBase->getColorValue(color);
            overlayColor.setAlpha(80);  // Прозрачность 80/255
            painter.fillRect(rect, overlayColor);

            // Рисуем вес в верхней половине клетки
            painter.setPen(Qt::black);
            QString weightText = QString::number(weight);

            QFont weightFont = painter.font();
            weightFont.setPointSize(qMax(10, m_cellSize / 6));  // Динамический размер шрифта
            weightFont.setBold(true);
            painter.setFont(weightFont);

            // Вес располагается в верхней половине клетки
            QRect weightRect = rect;
            weightRect.setHeight(rect.height() / 2);
            painter.drawText(weightRect, Qt::AlignCenter, weightText);

            // Для цвета используем меньший шрифт
            QFont colorFont = painter.font();
            colorFont.setPointSize(qMax(6, m_cellSize / 12));
            colorFont.setBold(false);
            painter.setFont(colorFont);
        }
    }
}

/**
 *  Рисует всех ферзей на доске
 *  painter Объект QPainter для рисования
 *
 * Проходит по всем столбцам и, если в столбце есть ферзь,
 * вызывает метод отрисовки одного ферзя.
 */
void ChessBoardWidget::drawQueens(QPainter& painter){
    for (int col = 0; col < m_boardSize; ++col) {
        if (col < static_cast<int>(m_queenPositions.size()) && m_queenPositions[col] >= 0) {
            int row = m_queenPositions[col];
            if (row < m_boardSize)
                drawQueenAt(painter, col, row);
        }
    }
}

/**
 *  Возвращает прямоугольник клетки в координатах виджета
 *  col Столбец (0-индексация)
 *  row Строка (0-индексация)
 */
QRect ChessBoardWidget::getCellRect(int col, int row) const{
    int x = col * m_cellSize;
    int y = row * m_cellSize;
    return QRect(x, y, m_cellSize, m_cellSize);
}

/**
 * Рисует одного ферзя в указанной клетке
 * painter Объект QPainter для рисования
 * col Столбец ферзя
 * row Строка ферзя
 *
 * Ферзь отображается как черный круг, занимающий 30% размера клетки.
 */
void ChessBoardWidget::drawQueenAt(QPainter& painter, int col, int row){
    QRect rect = getCellRect(col, row);
    int centerX = rect.center().x();
    int centerY = rect.center().y();
    int size = m_cellSize * 0.3;  // Размер ферзя относительно клетки

    painter.save();  // Сохраняем состояние painter'а
    painter.setBrush(QBrush(Qt::black));  // Черный цвет заливки
    painter.drawEllipse(centerX - size/2, centerY - size/2, size, size);
    painter.restore();  // Восстанавливаем состояние
}