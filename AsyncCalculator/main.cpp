#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include <QTimer>
#include "calculator.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

void printHelp(QTextStream& out)
{
    out << "Доступные команды:\n";
    out << "  add <a> <b>  - сложение (асинхронно)\n";
    out << "  sub <a> <b>  - вычитание (асинхронно)\n";
    out << "  mul <a> <b>  - умножение (асинхронно)\n";
    out << "  div <a> <b>  - деление (асинхронно)\n";
    out << "  reset        - сброс\n";
    out << "  stats        - статистика операций\n";
    out << "  version      - информация о программе\n";
    out << "  help         - эта справка\n";
    out << "  quit         - выход\n";
}

void printInfo(QTextStream& out)
{
    out << "=== Async Calculator on Qt ===\n";
    out << "Лабораторная работа №1. Продвинутый уровень, вариант 5.\n";
    out << "Асинхронный калькулятор с QThread и сигналами/слотами.\n";
    out << "Версия Qt: " << qVersion() << "\n";
}

int main(int argc, char* argv[])
{
#ifdef Q_OS_WIN
    // Без этих строк русский текст в консоли не будет видно
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    QCoreApplication app(argc, argv);

    // Потоки ввода/вывода. 
    QTextStream in(stdin);
    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    in.setEncoding(QStringConverter::Utf8);

    Calculator calc;

    // Подключаем сигналы калькулятора к обработчикам вывода.
    // Лямбды захватывают out по ссылке, чтобы писать в консоль.

    QObject::connect(&calc, &Calculator::resultReady, [&out](double result) {
        out << "Результат: " << result << "\n> ";
        out.flush();
        });

    QObject::connect(&calc, &Calculator::errorOccurred, [&out](const QString& msg) {
        out << "Ошибка: " << msg << "\n> ";
        out.flush();
        });

    QObject::connect(&calc, &Calculator::operationStarted, [&out](const QString& name) {
        out << "[" << name << "] операция запущена в отдельном потоке...\n";
        out.flush();
        });

    out << "=== Асинхронный калькулятор на Qt ===\n";
    out << "Лабораторная работа №1. Продвинутый уровень, вариант 5.\n";
    printHelp(out);
    out << "\n> ";
    out.flush();

    // Таймер опрашивает stdin каждые 50 мс — это не блокирует цикл событий
    // и позволяет одновременно обрабатывать сигналы из рабочих потоков.
    QTimer inputTimer;
    QObject::connect(&inputTimer, &QTimer::timeout, [&]() {
        if (in.atEnd()) {
            inputTimer.stop();
            return;
        }
        QString line;
        if (in.readLineInto(&line)) {
            line = line.trimmed();
            if (line.isEmpty()) {
                out << "> ";
                out.flush();
                return;
            }

            // Разбиваем строку на команду и операнды
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            QString command = parts.value(0).toLower();

           
            if (command == "quit" || command == "exit") {
                out << "До свидания!\n";
                out.flush();
                QCoreApplication::quit();
                return;
            }

            // Справка
            if (command == "help") {
                printHelp(out);
                out << "> ";
                out.flush();
                return;
            }

            // Информация о программе
            if (command == "version") {
                printInfo(out);
                out << "> ";
                out.flush();
                return;
            }

            // Статистика операций
            if (command == "stats") {
                out << "Выполнено операций: " << calc.operationCount() << "\n> ";
                out.flush();
                return;
            }

            // Сброс
            if (command == "reset") {
                calc.reset();
                return;
            }

            // Арифметические команды требуют ровно 3 токена
            if (parts.size() != 3) {
                out << "Ошибка: неверный формат. Используйте: <команда> <a> <b>\n> ";
                out.flush();
                return;
            }

            // Преобразование операндов в числа
            bool ok1, ok2;
            double a = parts[1].toDouble(&ok1);
            double b = parts[2].toDouble(&ok2);

            if (!ok1 || !ok2) {
                out << "Ошибка: не удалось преобразовать операнды в числа\n> ";
                out.flush();
                return;
            }

            // Запуск нужной операции
            if (command == "add") {
                calc.addAsync(a, b);
            }
            else if (command == "sub") {
                calc.subtractAsync(a, b);
            }
            else if (command == "mul") {
                calc.multiplyAsync(a, b);
            }
            else if (command == "div") {
                calc.divideAsync(a, b);
            }
            else {
                out << "Неизвестная команда: " << command << "\n> ";
                out.flush();
            }
        }
        });

    inputTimer.start(50);   // опрос ввода каждые 50 мс

    return app.exec();
}