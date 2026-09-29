#include "calculator.h"
#include <QDebug>
#include <QThread>

// ==================== Worker ====================

void Worker::process()
{
    double result = 0.0;

    switch (m_op) {
    case Add:      result = m_a + m_b; break;
    case Subtract: result = m_a - m_b; break;
    case Multiply: result = m_a * m_b; break;
    case Divide:   result = m_a / m_b; break;
    }

    qDebug() << "Worker thread id:" << QThread::currentThreadId()
        << "result =" << result;

    emit finished(result);
}

// ==================== Calculator ====================

Calculator::Calculator(QObject* parent)
    : QObject(parent)
    , m_result(0.0)
    , m_busy(false)
{
    qDebug() << "Calculator created";
}

Calculator::~Calculator()
{
    qDebug() << "Calculator destroyed";
}

void Calculator::runInThread(Worker::Operation op, const QString& name,
    double a, double b)
{
    if (m_busy) {
        emit errorOccurred("Предыдущая операция ещё выполняется!");
        return;
    }

    m_busy = true;
    emit operationStarted(name);

    QThread* thread = new QThread(this);
    Worker* worker = new Worker(op, a, b);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &Worker::process);
    connect(worker, &Worker::finished, this, [this, thread, worker](double value) {
        m_result = value;
        m_busy = false;
        emit resultReady(value);
        thread->quit();
        worker->deleteLater();
        });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void Calculator::addAsync(double a, double b)
{
    qDebug() << "addAsync(" << a << "," << b << ")";
    runInThread(Worker::Add, "add", a, b);
}

void Calculator::subtractAsync(double a, double b)
{
    qDebug() << "subtractAsync(" << a << "," << b << ")";
    runInThread(Worker::Subtract, "sub", a, b);
}

void Calculator::multiplyAsync(double a, double b)
{
    qDebug() << "multiplyAsync(" << a << "," << b << ")";
    runInThread(Worker::Multiply, "mul", a, b);
}

void Calculator::divideAsync(double a, double b)
{
    qDebug() << "divideAsync(" << a << "," << b << ")";

    if (qFuzzyIsNull(b)) {
        m_busy = false;
        emit errorOccurred("Деление на ноль невозможно!");
        return;
    }

    runInThread(Worker::Divide, "div", a, b);
}

void Calculator::reset()
{
    qDebug() << "reset()";
    m_result = 0.0;
    m_busy = false;
    emit resultReady(0.0);
}