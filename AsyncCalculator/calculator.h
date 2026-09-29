#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QObject>
#include <QString>

// Worker — выполняет одну арифметическую операцию в отдельном потоке.

class Worker : public QObject
{
    Q_OBJECT
public:
    enum Operation { Add, Subtract, Multiply, Divide };

    Worker(Operation op, double a, double b, QObject* parent = nullptr)
        : QObject(parent), m_op(op), m_a(a), m_b(b) {
    }

public slots:
    void process();   // точка входа, вызывается после старта потока

signals:
    void finished(double result);   // сигнал о завершении вычисления

private:
    Operation m_op;
    double m_a;
    double m_b;
};

// Calculator — управляет запуском асинхронных операций.
// Хранит последний результат и счётчик выполненных операций.
class Calculator : public QObject
{
    Q_OBJECT

public:
    explicit Calculator(QObject* parent = nullptr);
    ~Calculator();

    double result() const { return m_result; }
    bool isBusy() const { return m_busy; }
    int operationCount() const { return m_operationCount; }

public slots:
    void addAsync(double a, double b);
    void subtractAsync(double a, double b);
    void multiplyAsync(double a, double b);
    void divideAsync(double a, double b);
    void reset();

signals:
    void resultReady(double result);              // успешное вычисление
    void errorOccurred(const QString& message);   // любая ошибка
    void operationStarted(const QString& name);   // операция запущена

private:
    void runInThread(Worker::Operation op, const QString& name, double a, double b);

    double m_result;         // последний результат
    bool m_busy;             // true, пока операция выполняется
    int m_operationCount;    // сколько операций выполнено с момента запуска/reset
};

#endif 