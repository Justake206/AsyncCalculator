#include "calculator.h"
#include <QDebug>
#include <QThread>

void Worker::process()
{
    double result = 0.0;

    switch (m_op) {
    case Add:      result = m_a + m_b; break;
    case Subtract: result = m_a - m_b; break;
    case Multiply: result = m_a * m_b; break;
    case Divide:   result = m_a / m_b; break;
    }

    // Печатаем ID потока — доказательство асинхронности для отчёта
    qDebug() << "Worker thread id:" << QThread::currentThreadId()
        << "result =" << result;

    // Уведомляем главный поток о готовом результате
    emit finished(result);
}


// Конструктор: инициализируем состояние калькулятора
Calculator::Calculator(QObject* parent)
    : QObject(parent)
    , m_result(0.0)
    , m_busy(false)
    , m_operationCount(0)   // счётчик выполненных операций
{
    qDebug() << "Calculator created";
}

Calculator::~Calculator()
{
    qDebug() << "Calculator destroyed";
}

// Запускает операцию в отдельном потоке.
// Здесь создаётся QThread и Worker, между ними устанавливаются связи.
void Calculator::runInThread(Worker::Operation op, const QString& name,
    double a, double b)
{
    // Защита: пока выполняется одна операция, новую запускать нельзя
    if (m_busy) {
        emit errorOccurred("Предыдущая операция ещё выполняется!");
        return;
    }

    m_busy = true;
    ++m_operationCount;   // увеличиваем счётчик перед запуском
    emit operationStarted(name);

    // Создаём поток и рабочего, переносим рабочего в поток
    QThread* thread = new QThread(this);
    Worker* worker = new Worker(op, a, b);
    worker->moveToThread(thread);   // Worker будет работать в новом потоке

    // При старте потока автоматически вызовется Worker::process()
    connect(thread, &QThread::started, worker, &Worker::process);

    // Обработка результата — лямбда выполнится в главном потоке
    // (Qt сам обеспечивает потокобезопасную доставку сигнала).
    connect(worker, &Worker::finished, this, [this, thread, worker](double value) {
        m_result = value;              // сохраняем результат
        m_busy = false;                // освобождаем калькулятор
        emit resultReady(value);       // уведомляем подписчиков
        thread->quit();                // останавливаем поток
        worker->deleteLater();         // удаляем Worker после выхода
        });

    // Удаляем сам QThread после завершения
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

// Сброс: обнуляем результат, снимаем флаг занятости и счётчик операций
void Calculator::reset()
{
    qDebug() << "reset()";
    m_result = 0.0;
    m_busy = false;
    m_operationCount = 0;   // сбрасываем счётчик операций
    emit resultReady(0.0);
}