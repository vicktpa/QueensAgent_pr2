#ifndef QUEENSSOLVER_H
#define QUEENSSOLVER_H

#include <QObject>
#include <vector>
#include <atomic>
#include <map>
#include <set>
#include "KnowledgeBase.h"

/**
 *  Структура для хранения решения вместе с его весом и цветами
 *
 * Расширяет базовое представление решения (вектор позиций)
 * дополнительной информацией для сортировки и анализа.
 */
struct SolutionWithCost {
    std::vector<int> positions;              // Позиции ферзей (индекс - столбец, значение - строка)
    int totalWeight;                         // Суммарный вес решения
    std::map<int, PositionColor> colorMap;   // Цвет каждой позиции (столбец -> цвет)

    // Операторы сравнения для сортировки по весу
    bool operator<(const SolutionWithCost& other) const {
        return totalWeight < other.totalWeight;
    }

    bool operator>(const SolutionWithCost& other) const {
        return totalWeight > other.totalWeight;
    }
};

/**
 *  Класс для решения задачи о ферзях
 *
 * Основные возможности:
 * 1. Поиск всех решений задачи о N ферзях
 * 2. Учет базы знаний (веса и цвета клеток)
 * 3. Сортировка решений по весу
 * 4. Асинхронное выполнение в отдельном потоке
 *
 * Алгоритм поиска: рекурсивный backtracking с отсечениями
 * - Проверка атак по горизонтали и диагоналям
 * - Проверка цветовой совместимости (при использовании БЗ)
 */
class QueensSolver : public QObject
{
    Q_OBJECT

public:
    /**
     *  Конструктор для базового использования (без БЗ)
     *  boardSize Размер доски
     *  parent Родительский объект
     */
    explicit QueensSolver(int boardSize, QObject* parent = nullptr);

    /**
     *  Конструктор с поддержкой базы знаний
     *  boardSize Размер доски
     *  knowledgeBase Указатель на базу знаний
     *  parent Родительский объект
     */
    explicit QueensSolver(int boardSize, KnowledgeBase* knowledgeBase,
                          QObject* parent = nullptr);

    ~QueensSolver();

    /**
     *  Пересчитывает веса для всех решений
     *
     * Вызывается после изменения весов в базе знаний.
     */
    void recalculateAllWeights();

public slots:
    /**
     *  Запускает поиск всех решений
     *
     * Выполняется асинхронно, результаты возвращаются через сигналы.
     */
    void solve();

    /**
     *  Останавливает текущий поиск
     */
    void stop();

    /**
     *  Сортирует найденные решения по весу
     *  ascending true - по возрастанию, false - по убыванию
     *  sortType Тип сортировки (зарезервирован для будущих расширений)
     */
    void sortSolutionsByCost(bool ascending = true, int sortType = 0);

    /**
     *  Возвращает все найденные решения (только позиции)
     */
    const std::vector<std::vector<int>>& getAllSolutions() const { return m_allSolutions; }

    /**
     *  Возвращает все решения с весами и цветами
     */
    const std::vector<SolutionWithCost>& getSolutionsWithCost() const { return m_solutionsWithCost; }

signals:
    /**  Сигнал о нахождении нового решения (только позиции) */
    void solutionFound(const std::vector<int>& solution);

    /**  Сигнал о нахождении нового решения (с весом и цветами) */
    void solutionWithCostFound(const SolutionWithCost& solution);

    /**  Сигнал о прогрессе поиска */
    void progressUpdated(int current, int total);

    /**  Сигнал о завершении поиска */
    void finished(int totalSolutions);

    /**  Сигнал об ошибке */
    void error(const QString& error);

    /**  Сигнал для вывода отформатированного решения в консоль */
    void solutionPrinted(const QString& solutionText);

    /**  Сигнал о том, что все решения найдены и загружены */
    void allSolutionsFound(const std::vector<std::vector<int>>& solutions);

    /**  Сигнал о готовности всех решений с весами */
    void allSolutionsWithCostReady(const std::vector<SolutionWithCost>& solutions);

    /**  Сигнал о завершении сортировки */
    void sortingCompleted(int count, bool ascending);

private:
    /**
     *  Рекурсивный поиск всех решений
     *  col Текущий столбец
     *  board Текущая расстановка ферзей
     *  uniqueSolutions Множество уникальных решений
     *  solutionCount Счетчик найденных решений
     *  expectedCount Ожидаемое количество решений
     */
    void findAllSolutions(int col, std::vector<int>& board,
                          std::set<std::vector<int>>& uniqueSolutions,
                          int& solutionCount, int expectedCount);

    /**
     *  Проверяет, безопасна ли позиция для ферзя
     *  col Столбец
     *  row Строка
     *  board Текущая расстановка
     */
    bool isSafe(int col, int row, const std::vector<int>& board) const;

    /**
     *  Проверяет цветовую совместимость с уже расставленными ферзями
     *  col Столбец
     *  row Строка
     *  board Текущая расстановка
     */
    bool isColorCompatible(int col, int row, const std::vector<int>& board) const;

    /**
     *  Форматирует решение для вывода в консоль
     *  solution Решение
     *  solutionNumber Номер решения
     */
    QString formatSolution(const std::vector<int>& solution, int solutionNumber = 0) const;

    /**
     *  Возвращает ожидаемое количество решений для заданного N
     *  n Размер доски
     */
    int getExpectedSolutionCount(int n) const;

    /**
     *  Проверяет, все ли ферзи в решении одного цвета
     *  solution Решение
     */
    bool validateColorCompatibility(const std::vector<int>& solution) const;

    /**
     *  Вычисляет вес для решения
     *  solution Структура решения (будет обновлена)
     */
    void calculateCostsForSolution(SolutionWithCost& solution) const;

    /**
     *  Получает цвета всех ферзей в решении
     *  solution Решение (позиции)
     */
    std::map<int, PositionColor> getColorsForSolution(const std::vector<int>& solution) const;

private:
    int m_boardSize;                                    // Размер доски
    std::vector<std::vector<int>> m_allSolutions;      // Все найденные решения
    std::atomic<bool> m_stopRequested;                 // Флаг остановки поиска
    std::atomic<bool> m_isSolving;                     // Флаг выполнения поиска

    KnowledgeBase* m_knowledgeBase;                    // База знаний
    std::vector<SolutionWithCost> m_solutionsWithCost; // Решения с весами
    bool m_useKnowledgeBase;                           // Использовать ли БЗ

    /**
     *  Статическая таблица ожидаемых количеств решений
     *
     * Для известных значений N (1-15) хранит точное количество решений.
     * Используется для отображения прогресса.
     */
    static const std::map<int, int> s_expectedSolutions;
};

#endif // QUEENSSOLVER_H