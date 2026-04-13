#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QProgressBar>
#include <QThread>
#include <QTextEdit>
#include <QCloseEvent>
#include <QTimer>
#include <QCheckBox>
#include <QScrollArea>
#include "KnowledgeBase.h"

// Forward declarations для избежания циклических зависимостей
class ChessBoardWidget;
class QueensSolver;
class KnowledgeBase;

/**
 *  Главное окно приложения "Задача о ферзях"
 *
 * Основной класс, управляющий всем приложением:
 * - Организация пользовательского интерфейса
 * - Управление поиском решений в отдельном потоке
 * - Навигация по найденным решениям
 * - Взаимодействие с базой знаний
 * - Отображение консоли с решениями
 *
 * Архитектура:
 * - Слева: шахматная доска и панели управления
 * - Справа: консоль с текстовым выводом решений
 *
 * Поиск решений выполняется в отдельном потоке (QThread + QueensSolver),
 * чтобы не блокировать пользовательский интерфейс.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     *  Конструктор главного окна
     *  parent Родительский виджет
     *
     * Инициализирует все компоненты, создает интерфейс,
     * подключает сигналы и слоты.
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     *  Деструктор
     *
     * Останавливает поиск, завершает поток и освобождает ресурсы.
     */
    ~MainWindow();

protected:
    /**
     *  Обработчик закрытия окна
     *  event Событие закрытия
     *
     * Останавливает анимацию и поиск перед закрытием.
     */
    void closeEvent(QCloseEvent* event) override;

    /**
     *  Обработчик изменения размера окна
     *  event Событие изменения размера
     *
     * Адаптирует размер консоли под новое окно.
     */
    void resizeEvent(QResizeEvent* event) override;

private slots:
    // ========================================================================
    // Управление поиском решений
    // ========================================================================
    void startSolving();                    ///< Начать поиск решений
    void stopSolving();                     ///< Остановить поиск решений
    void updateProgress(int current, int total);  ///< Обновить прогресс-бар

    // ========================================================================
    // Обработка результатов поиска
    // ========================================================================
    void onSolutionFound(const std::vector<int>& solution);     ///< Найдено новое решение
    void onSolutionPrinted(const QString& solutionText);        ///< Вывод решения в консоль
    void onSolvingFinished(int totalSolutions);                 ///< Поиск завершен
    void onError(const QString& errorMessage);                  ///< Ошибка при поиске
    void onAllSolutionsFound(const std::vector<std::vector<int>>& solutions); ///< Все решения найдены

    // ========================================================================
    // Навигация по решениям
    // ========================================================================
    void nextSolution();        ///< Следующее решение
    void previousSolution();    ///< Предыдущее решение
    void firstSolution();       ///< Первое решение
    void showAllSolutions();    ///< Показать все решения в анимации
    void stopAnimation();       ///< Остановить анимацию
    void animateSolution(int index);  ///< Анимация показа решений

    // ========================================================================
    // Управление консолью
    // ========================================================================
    void clearConsole();        ///< Очистить консоль
    void saveConsoleToFile();   ///< Сохранить консоль в файл

    // ========================================================================
    // Управление базой знаний
    // ========================================================================
    void openKnowledgeBaseDialog();     ///< Открыть диалог управления БЗ
    void openSortDialog();              ///< Открыть диалог сортировки
    void onSolutionsSorted(bool ascending);  ///< Обработка сортировки решений
    void updateSolutionInfoWithWeight();     ///< Обновить информацию о весе решения
    void updateCurrentSolutionDisplay();     ///< Обновить отображение текущего решения

    void toggleEditMode(bool enabled);       ///< Включить/выключить режим редактирования
    void randomizeAllKnowledge();            ///< Случайная генерация БЗ (2 цвета)
    void resetAllKnowledge();                ///< Сброс БЗ (все зеленые)
    void generateDefaultAllGreen();          ///< Сгенерировать все зеленые клетки
    void generateDefaultTwoColorDistribution(); ///< Сгенерировать 2-цветное распределение

    // ========================================================================
    // Управление доской
    // ========================================================================
    void applyBoardSize();                  ///< Применить новый размер доски

    // ========================================================================
    // Обработка изменений в базе знаний
    // ========================================================================
    void onCellWeightChanged(int col, int row, int newWeight);  ///< Изменен вес клетки
    void onCellColorChanged(int col, int row, PositionColor newColor); ///< Изменен цвет клетки

private:
    /**
     *  Настройка пользовательского интерфейса
     *
     * Создает все виджеты, размещает их в layout'ах,
     * настраивает размеры и свойства.
     */
    void setupUI();

    /**
     *  Подключение сигналов и слотов
     *
     * Связывает сигналы кнопок, виджетов и слотов.
     */
    void connectSignals();

    /**
     *  Вывод текста в консоль
     *  text Текст для вывода
     *
     * Добавляет текст в QTextEdit и автоматически прокручивает вниз.
     */
    void appendToConsole(const QString& text);

private:
    // ========================================================================
    // Виджеты левой панели (доска и управление)
    // ========================================================================
    ChessBoardWidget* m_boardWidget;           ///< Виджет шахматной доски

    // Группа "Поиск решений"
    QPushButton* m_startButton;                ///< Кнопка "Начать поиск"
    QPushButton* m_stopButton;                 ///< Кнопка "Остановить поиск"
    QSpinBox* m_boardSizeSpinBox;              ///< Поле ввода размера доски
    QPushButton* m_applySizeButton;            ///< Кнопка "Применить размер"
    QLabel* m_solutionsCountLabel;             ///< Метка с количеством решений
    QProgressBar* m_progressBar;               ///< Прогресс-бар поиска

    // Группа "Отображение"
    QCheckBox* m_editModeCheckBox;             ///< Чекбокс "Режим редактирования"

    // Группа "База знаний"
    QPushButton* m_knowledgeBaseButton;        ///< Кнопка "Управлять базой знаний"
    QPushButton* m_sortSolutionsButton;        ///< Кнопка "Сортировать"
    QPushButton* m_randomizeKnowledgeButton;   ///< Кнопка "Случайно (2 цвета)"
    QPushButton* m_resetKnowledgeButton;       ///< Кнопка "Сбросить"

    // Группа "Навигация"
    QPushButton* m_firstButton;                ///< Кнопка "Первое решение"
    QPushButton* m_prevButton;                 ///< Кнопка "Предыдущее"
    QPushButton* m_nextButton;                 ///< Кнопка "Следующее"
    QPushButton* m_showAllButton;              ///< Кнопка "Показать все"
    QPushButton* m_stopAnimationButton;        ///< Кнопка "Остановить показ"

    // ========================================================================
    // Виджеты правой панели (консоль)
    // ========================================================================
    QTextEdit* m_consoleTextEdit;              ///< Текстовое поле консоли
    QPushButton* m_clearConsoleButton;         ///< Кнопка "Очистить"
    QPushButton* m_saveConsoleButton;          ///< Кнопка "Сохранить в файл"

    // ========================================================================
    // Поиск решений (поток)
    // ========================================================================
    QueensSolver* m_solver;                    ///< Объект решателя
    QThread* m_solverThread;                   ///< Поток для выполнения поиска
    KnowledgeBase* m_knowledgeBase;            ///< База знаний

    // ========================================================================
    // Состояние приложения
    // ========================================================================
    bool m_isSolving;                          ///< Выполняется ли поиск
    int m_currentSolutionsCount;               ///< Текущее количество найденных решений
    int m_currentAnimationIndex;               ///< Текущий индекс в анимации
    bool m_animationRunning;                   ///< Выполняется ли анимация
};

#endif // MAINWINDOW_H