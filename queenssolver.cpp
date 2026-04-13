#include "QueensSolver.h"
#include <QThread>
#include <QElapsedTimer>
#include <QDebug>
#include <iostream>
#include <iomanip>
#include <set>
#include <algorithm>
#include <cmath>

// ============================================================================
// Статические данные
// ============================================================================

/**
 *  Таблица ожидаемых количеств решений для N от 1 до 15
 *
 * Источник: известные значения для задачи о N ферзях
 */
const std::map<int, int> QueensSolver::s_expectedSolutions = {
    {1, 1}, {2, 0}, {3, 0}, {4, 2}, {5, 10}, {6, 4}, {7, 40},
    {8, 92}, {9, 352}, {10, 724}, {11, 2680}, {12, 14200},
    {13, 73712}, {14, 365596}, {15, 2279184}
};

// ============================================================================
// Конструкторы и деструктор
// ============================================================================

QueensSolver::QueensSolver(int boardSize, QObject* parent)
    : QObject(parent)
    , m_boardSize(boardSize)
    , m_stopRequested(false)
    , m_isSolving(false)
    , m_knowledgeBase(nullptr)
    , m_useKnowledgeBase(false)
{
}

QueensSolver::QueensSolver(int boardSize, KnowledgeBase* knowledgeBase, QObject* parent)
    : QObject(parent)
    , m_boardSize(boardSize)
    , m_stopRequested(false)
    , m_isSolving(false)
    , m_knowledgeBase(knowledgeBase)
    , m_useKnowledgeBase(true)
{
}

QueensSolver::~QueensSolver() {
    stop();
}

// ============================================================================
// Публичные методы
// ============================================================================

/**
 *  Возвращает ожидаемое количество решений для заданного N
 *  n Размер доски
 */
int QueensSolver::getExpectedSolutionCount(int n) const
{
    auto it = s_expectedSolutions.find(n);
    return (it != s_expectedSolutions.end()) ? it->second : 0;
}

/**
 *  Вычисляет вес для решения
 *  solution Структура решения (будет обновлена)
 *
 * Суммирует веса всех клеток, на которых стоят ферзи.
 */
void QueensSolver::calculateCostsForSolution(SolutionWithCost& solution) const
{
    if (!m_knowledgeBase) return;

    solution.totalWeight = 0;

    for (int col = 0; col < static_cast<int>(solution.positions.size()); ++col) {
        if (solution.positions[col] != -1) {
            int row = solution.positions[col] + 1;  // В БЗ строки с 1
            int weight = m_knowledgeBase->getPositionWeight(col, row);
            solution.totalWeight += weight;
        }
    }
}

/**
 *  Запускает поиск всех решений
 *
 * Алгоритм:
 * 1. Очищает предыдущие результаты
 * 2. Запускает рекурсивный backtracking
 * 3. Собирает уникальные решения в множество
 * 4. Если используется БЗ - вычисляет веса и цвета
 * 5. Отправляет результаты через сигналы
 */
void QueensSolver::solve()
{
    if (m_isSolving) return;

    m_stopRequested = false;
    m_isSolving = true;
    m_allSolutions.clear();
    m_solutionsWithCost.clear();

    int expectedCount = getExpectedSolutionCount(m_boardSize);

    // Начальная доска: все клетки пусты (-1)
    std::vector<int> board(m_boardSize, -1);
    std::set<std::vector<int>> uniqueSolutions;  // Множество для уникальности

    QElapsedTimer timer;
    timer.start();
    const int MAX_TIME_MS = 120000;  // Максимальное время поиска: 2 минуты

    int solutionCount = 0;

    // Запускаем рекурсивный поиск
    findAllSolutions(0, board, uniqueSolutions, solutionCount, expectedCount);

    // Проверка на превышение времени
    if (timer.elapsed() > MAX_TIME_MS) {
        Q_EMIT error("Превышено время поиска");
    }

    // Преобразуем множество в вектор
    m_allSolutions.assign(uniqueSolutions.begin(), uniqueSolutions.end());

    // Если используем БЗ - добавляем веса и цвета
    if (m_useKnowledgeBase && m_knowledgeBase) {
        m_solutionsWithCost.clear();
        for (const auto& sol : m_allSolutions) {
            SolutionWithCost solutionWithCost;
            solutionWithCost.positions = sol;
            solutionWithCost.colorMap = getColorsForSolution(sol);
            calculateCostsForSolution(solutionWithCost);
            m_solutionsWithCost.push_back(solutionWithCost);
        }
        Q_EMIT allSolutionsWithCostReady(m_solutionsWithCost);
    }

    // Отправляем результаты
    if (!m_allSolutions.empty()) {
        Q_EMIT allSolutionsFound(m_allSolutions);
    }

    Q_EMIT finished(m_allSolutions.size());
    m_isSolving = false;
}

/**
 *  Рекурсивный поиск всех решений
 *  col Текущий столбец
 *  board Текущая расстановка
 *  uniqueSolutions Множество уникальных решений
 *  solutionCount Счетчик решений
 *  expectedCount Ожидаемое количество
 *
 * Алгоритм backtracking:
 * - Если col == boardSize: нашли полное решение
 * - Иначе: перебираем все строки для текущего столбца
 * - Для каждой строки проверяем безопасность и цветовую совместимость
 * - Если подходит - ставим ферзя и рекурсивно идем дальше
 */
void QueensSolver::findAllSolutions(int col, std::vector<int>& board,
                                    std::set<std::vector<int>>& uniqueSolutions,
                                    int& solutionCount, int expectedCount)
{
    // Проверка остановки по запросу пользователя
    if (m_stopRequested) return;

    // Базовый случай: все ферзи расставлены
    if (col == m_boardSize) {
        // Проверка цветовой совместимости (если используем БЗ)
        bool sameColor = true;
        if (m_useKnowledgeBase && m_knowledgeBase && m_boardSize > 0) {
            // Находим первого ферзя для определения эталонного цвета
            int firstCol = -1;
            for (int i = 0; i < m_boardSize; i++) {
                if (board[i] != -1) {
                    firstCol = i;
                    break;
                }
            }
            if (firstCol != -1) {
                PositionColor firstColor = m_knowledgeBase->getPositionColor(firstCol, board[firstCol] + 1);
                // Проверяем, что все ферзи того же цвета
                for (int i = firstCol + 1; i < m_boardSize; i++) {
                    if (board[i] != -1) {
                        PositionColor color = m_knowledgeBase->getPositionColor(i, board[i] + 1);
                        if (color != firstColor) {
                            sameColor = false;
                            break;
                        }
                    }
                }
            }
        }

        // Если решение валидно и уникально - сохраняем
        if (sameColor && uniqueSolutions.find(board) == uniqueSolutions.end()) {
            uniqueSolutions.insert(board);
            solutionCount++;

            // Отправляем сигналы
            Q_EMIT solutionFound(board);
            Q_EMIT progressUpdated(solutionCount, expectedCount);

            QString solutionText = formatSolution(board, solutionCount);
            Q_EMIT solutionPrinted(solutionText);
        }
        return;
    }

    // Рекурсивный случай: перебираем все строки для текущего столбца
    for (int row = 0; row < m_boardSize; row++) {
        if (m_stopRequested) return;

        // Проверка безопасности (атаки)
        if (isSafe(col, row, board)) {
            // Проверка цветовой совместимости (если используем БЗ)
            if (m_useKnowledgeBase && m_knowledgeBase) {
                if (!isColorCompatible(col, row, board)) {
                    continue;  // Цвет не совместим - пропускаем
                }
            }

            // Ставим ферзя и рекурсивно идем дальше
            board[col] = row;
            findAllSolutions(col + 1, board, uniqueSolutions, solutionCount, expectedCount);
            board[col] = -1;  // Убираем ферзя (backtrack)
        }
    }
}

/**
 *  Проверяет, безопасна ли позиция для ферзя
 *  col Столбец
 *  row Строка
 *  board Текущая расстановка
 *
 * Проверяет:
 * 1. Нет ли другого ферзя в той же строке
 * 2. Нет ли другого ферзя на той же диагонали
 */
bool QueensSolver::isSafe(int col, int row, const std::vector<int>& board) const
{
    for (int prevCol = 0; prevCol < col; prevCol++) {
        int prevRow = board[prevCol];
        if (prevRow == -1) continue;

        // Проверка горизонтали
        if (prevRow == row) {
            return false;
        }
        // Проверка диагоналей
        if (std::abs(prevRow - row) == std::abs(prevCol - col)) {
            return false;
        }
    }
    return true;
}

/**
 *  Проверяет цветовую совместимость с уже расставленными ферзями
 *  col Столбец
 *  row Строка
 *  board Текущая расстановка
 *
 * Условие: все ферзи в решении должны быть одного цвета.
 * Проверяем, что цвет нового ферзя совпадает с цветом
 * уже расставленных ферзей.
 */
bool QueensSolver::isColorCompatible(int col, int row, const std::vector<int>& board) const
{
    if (!m_knowledgeBase) return true;

    PositionColor currentColor = m_knowledgeBase->getPositionColor(col, row + 1);

    // Проверяем всех уже расставленных ферзей
    for (int prevCol = 0; prevCol < col; prevCol++) {
        if (board[prevCol] != -1) {
            PositionColor prevColor = m_knowledgeBase->getPositionColor(prevCol, board[prevCol] + 1);
            if (currentColor != prevColor) {
                return false;  // Найден ферзь другого цвета
            }
        }
    }

    return true;
}

/**
 *  Останавливает текущий поиск
 *
 * Устанавливает флаг остановки и ждет завершения
 * рекурсивных вызовов (до 1 секунды).
 */
void QueensSolver::stop()
{
    m_stopRequested = true;
    int waitCount = 0;
    while (m_isSolving && waitCount < 100) {
        QThread::msleep(10);
        waitCount++;
    }
}

/**
 *  Форматирует решение для вывода в консоль
 *  solution Решение
 *  solutionNumber Номер решения
 */
QString QueensSolver::formatSolution(const std::vector<int>& solution, int solutionNumber) const
{
    QString result;

    // Заголовок решения
    if (solutionNumber > 0) {
        result = QString("\n========================================");
        result += QString("\nРЕШЕНИЕ #%1").arg(solutionNumber);
        result += "\n========================================\n";
    }

    // Вывод позиций (формат: A1, B5, C8, ...)
    result += "Позиции: ";
    for (int i = 0; i < static_cast<int>(solution.size()); ++i) {
        if (solution[i] != -1) {
            char colLetter = 'A' + static_cast<char>(i);
            result += QString("%1%2").arg(colLetter).arg(solution[i] + 1);
            if (i < static_cast<int>(solution.size()) - 1) result += ", ";
        }
    }
    result += "\n";

    // Вывод веса и цвета (если используем БЗ)
    if (m_useKnowledgeBase && m_knowledgeBase && !solution.empty()) {
        int totalWeight = 0;

        for (int col = 0; col < static_cast<int>(solution.size()); ++col) {
            if (solution[col] != -1) {
                int row = solution[col] + 1;
                int weight = m_knowledgeBase->getPositionWeight(col, row);
                totalWeight += weight;
            }
        }

        result += QString("Общий вес решения: %1\n").arg(totalWeight);

        // Определяем цвет решения
        int firstCol = -1;
        for (int i = 0; i < static_cast<int>(solution.size()); i++) {
            if (solution[i] != -1) {
                firstCol = i;
                break;
            }
        }
        if (firstCol != -1) {
            PositionColor color = m_knowledgeBase->getPositionColor(firstCol, solution[firstCol] + 1);
            QString colorName;
            switch (color) {
            case PositionColor::Red:   colorName = "Красный"; break;
            case PositionColor::Green: colorName = "Зеленый"; break;
            case PositionColor::Blue:  colorName = "Синий"; break;
            }
            result += QString("Цвет решения: %1\n").arg(colorName);
        }
    }

    result += "----------------------------------------\n";

    return result;
}

/**
 *  Проверяет, все ли ферзи в решении одного цвета
 *  solution Решение
 */
bool QueensSolver::validateColorCompatibility(const std::vector<int>& solution) const
{
    if (!m_knowledgeBase) return true;
    if (solution.empty()) return false;

    // Находим первого ферзя для эталонного цвета
    int firstCol = -1;
    for (int i = 0; i < static_cast<int>(solution.size()); i++) {
        if (solution[i] != -1) {
            firstCol = i;
            break;
        }
    }
    if (firstCol == -1) return false;

    int firstRow = solution[firstCol] + 1;
    PositionColor requiredColor = m_knowledgeBase->getPositionColor(firstCol, firstRow);

    // Проверяем всех ферзей
    for (int col = 0; col < static_cast<int>(solution.size()); ++col) {
        if (solution[col] != -1) {
            int row = solution[col] + 1;
            PositionColor color = m_knowledgeBase->getPositionColor(col, row);
            if (color != requiredColor) {
                return false;
            }
        }
    }
    return true;
}

/**
 *  Получает цвета всех ферзей в решении
 *  solution Решение (позиции)
 */
std::map<int, PositionColor> QueensSolver::getColorsForSolution(const std::vector<int>& solution) const
{
    std::map<int, PositionColor> colors;
    if (!m_knowledgeBase) return colors;

    for (int col = 0; col < static_cast<int>(solution.size()); ++col) {
        if (solution[col] != -1) {
            int row = solution[col] + 1;
            colors[col] = m_knowledgeBase->getPositionColor(col, row);
        }
    }
    return colors;
}

/**
 *  Сортирует найденные решения по весу
 *  ascending true - по возрастанию, false - по убыванию
 *  sortType Тип сортировки (зарезервирован)
 */
void QueensSolver::sortSolutionsByCost(bool ascending, int sortType)
{
    if (!m_useKnowledgeBase || m_solutionsWithCost.empty()) return;

    // Сортируем по totalWeight
    if (ascending) {
        std::sort(m_solutionsWithCost.begin(), m_solutionsWithCost.end(),
                  [](const SolutionWithCost& a, const SolutionWithCost& b) {
                      return a.totalWeight < b.totalWeight;
                  });
    } else {
        std::sort(m_solutionsWithCost.begin(), m_solutionsWithCost.end(),
                  [](const SolutionWithCost& a, const SolutionWithCost& b) {
                      return a.totalWeight > b.totalWeight;
                  });
    }

    // Обновляем вектор позиций в соответствии с отсортированным порядком
    m_allSolutions.clear();
    for (const auto& sol : m_solutionsWithCost) {
        m_allSolutions.push_back(sol.positions);
    }

    Q_EMIT sortingCompleted(m_solutionsWithCost.size(), ascending);
}

/**
 *  Пересчитывает веса для всех решений
 *
 * Вызывается после изменения весов в базе знаний.
 */
void QueensSolver::recalculateAllWeights()
{
    if (!m_useKnowledgeBase || !m_knowledgeBase) return;

    for (auto& sol : m_solutionsWithCost) {
        int totalWeight = 0;
        for (int col = 0; col < static_cast<int>(sol.positions.size()); ++col) {
            if (sol.positions[col] != -1) {
                int row = sol.positions[col] + 1;
                totalWeight += m_knowledgeBase->getPositionWeight(col, row);
            }
        }
        sol.totalWeight = totalWeight;
    }

    Q_EMIT sortingCompleted(m_solutionsWithCost.size(), true);
}