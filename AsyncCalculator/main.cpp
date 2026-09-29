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
    out << "  help         - эта справка\n";
    out << "  quit         - выход\n";
}

int main(int argc, char* argv[])
{
#ifdef Q_OS_WIN
    // Переключаем консоль Windows в режим UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    QCoreApplication app(argc, argv);

    QTextStream in(stdin);
    QTextStream out(stdout);
    out.setEncoding(QStringConverter::Utf8);
    in.setEncoding(QStringConverter::Utf8);

    Calculator calc;

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
    printHelp(out);
    out << "\n> ";
    out.flush();

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

            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            QString command = parts.value(0).toLower();

            if (command == "quit" || command == "exit") {
                out << "До свидания!\n";
                out.flush();
                QCoreApplication::quit();
                return;
            }

            if (command == "help") {
                printHelp(out);
                out << "> ";
                out.flush();
                return;
            }

            if (command == "reset") {
                calc.reset();
                return;
            }

            if (parts.size() != 3) {
                out << "Ошибка: неверный формат. Используйте: <команда> <a> <b>\n> ";
                out.flush();
                return;
            }

            bool ok1, ok2;
            double a = parts[1].toDouble(&ok1);
            double b = parts[2].toDouble(&ok2);

            if (!ok1 || !ok2) {
                out << "Ошибка: не удалось преобразовать операнды в числа\n> ";
                out.flush();
                return;
            }

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

    inputTimer.start(50);

    return app.exec();
}