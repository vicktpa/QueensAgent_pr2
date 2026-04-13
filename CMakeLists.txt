cmake_minimum_required(VERSION 3.16)

project(pr1 VERSION 0.1 LANGUAGES CXX)

# Настройки C++ стандарта
set(CMAKE_CXX_STANDARD 17)              # Используем C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Обязательно использовать C++17
set(CMAKE_CXX_EXTENSIONS OFF)           # Отключаем расширения компилятора

# Автоматическая обработка Qt-специфичных файлов
set(CMAKE_AUTOMOC ON)   # Автоматическая генерация кода для Q_OBJECT макроса
set(CMAKE_AUTORCC ON)   # Автоматическая компиляция ресурсов (.qrc)
set(CMAKE_AUTOUIC ON)   # Автоматическая компиляция UI-файлов (.ui)

# Поиск необходимых компонентов Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Sql)

# Настройка стандартного Qt-проекта
qt_standard_project_setup()

# Список исходных файлов
set(SOURCES
    main.cpp
    queenagent.cpp
    chessboardwidget.cpp
    mainwindow.cpp
    queenssolver.cpp
)

# Список заголовочных файлов
set(HEADERS
    queenagent.h
    chessboardwidget.h
    mainwindow.h
    queenssolver.h
)

qt_add_executable(apppr1 ${SOURCES} ${HEADERS}
    knowledgebase.h knowledgebase.cpp
    knowledgebasedialog.h knowledgebasedialog.cpp
    sortdialog.h sortdialog.cpp)

# Платформо-зависимые настройки
if(APPLE)
    # macOS: создаём bundle (.app)
    set_target_properties(apppr1 PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.queensagent.app"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
elseif(WIN32)
    # Windows: GUI приложение без консольного окна
    set_target_properties(apppr1 PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Подключение библиотек Qt
target_link_libraries(apppr1
    PRIVATE Qt6::Core    # Ядро Qt (сигналы/слоты, события)
    PRIVATE Qt6::Widgets # Виджеты Qt (окна, кнопки)
    PRIVATE Qt6::Gui     # Графические компоненты Qt
    PRIVATE Qt6::Sql
)

install(TARGETS apppr1
    BUNDLE DESTINATION . # Для macOS
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # Для библиотек
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # Для Windows/Linux
)