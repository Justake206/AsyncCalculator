#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QObject>
#include <QString>

class Worker : public QObject
{
    Q_OBJECT
public:
    enum Operation { Add, Subtract, Multiply, Divide };

    Worker(Operation op, double a, double b, QObject* parent = nullptr)
        : QObject(parent), m_op(op), m_a(a), m_b(b) {
    }

public slots:
    void process();

signals:
    void finished(double result);

private:
    Operation m_op;
    double m_a;
    double m_b;
};

class Calculator : public QObject
{
    Q_OBJECT

public:
    explicit Calculator(QObject* parent = nullptr);
    ~Calculator();

    double result() const { return m_result; }
    bool isBusy() const { return m_busy; }

public slots:
    void addAsync(double a, double b);
    void subtractAsync(double a, double b);
    void multiplyAsync(double a, double b);
    void divideAsync(double a, double b);
    void reset();

signals:
    void resultReady(double result);
    void errorOccurred(const QString& message);
    void operationStarted(const QString& name);

private:
    void runInThread(Worker::Operation op, const QString& name, double a, double b);

    double m_result;
    bool m_busy;
};

#endif // CALCULATOR_H