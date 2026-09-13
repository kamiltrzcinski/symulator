#include "thales/thales_window.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QShortcut>
#include <QDebug>

ThalesElementItem::ThalesElementItem(const QString& id, qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent)
    : QObject(), QGraphicsRectItem(x, y, w, h, parent), m_id(id), m_highlighted(false) {
    setBrush(Qt::gray);
    setPen(QPen(Qt::black, 2));
    setAcceptHoverEvents(true);
}

void ThalesElementItem::setHighlight(bool active, const QColor& color) {
    m_highlighted = active;
    if (active) {
        setPen(QPen(color, 4));
    } else {
        setPen(QPen(Qt::black, 2));
    }
}

void ThalesElementItem::setPathColor(const QColor& color) {
    setBrush(color);
}

void ThalesElementItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit elementClicked(m_id);
    }
    QGraphicsRectItem::mousePressEvent(event);
}

ThalesWindow::ThalesWindow(QWidget* parent)
    : QMainWindow(parent), m_state(State::IDLE) {
    setupUi();
    
    m_specTimer = new QTimer(this);
    m_specTimer->setSingleShot(true);
    connect(m_specTimer, &QTimer::timeout, this, &ThalesWindow::onSpecTimeout);

    // KROK 3: Wybór rodzaju przebiegu (Przyciski)
    connect(m_btnPOC, &QPushButton::clicked, this, [this]() { onCommandButtonClicked("POC"); });
    connect(m_btnMAN, &QPushButton::clicked, this, [this]() { onCommandButtonClicked("MAN"); });
    connect(m_btnSZ, &QPushButton::clicked, this, [this]() { onCommandButtonClicked("SZ"); });
    connect(m_btnDPZ, &QPushButton::clicked, this, [this]() { onCommandButtonClicked("DPZ"); });
    connect(m_btnZW, &QPushButton::clicked, this, [this]() { onCommandButtonClicked("ZW"); });
    
    connect(m_btnP, &QPushButton::clicked, this, &ThalesWindow::onEnterPressed);
    connect(m_inputLine, &QLineEdit::returnPressed, this, &ThalesWindow::onEnterPressed);
    
    connect(m_btnSPEC, &QPushButton::clicked, this, &ThalesWindow::onSpecButtonClicked);
    
    QShortcut* specShortcut = new QShortcut(QKeySequence("Ctrl+A"), this);
    connect(specShortcut, &QShortcut::activated, this, &ThalesWindow::onSpecButtonClicked);
    
    // Add dummy elements to the scene to represent semaphores/switches
    auto* s1 = new ThalesElementItem("S1", 0, 50, 40, 40);
    auto* s2 = new ThalesElementItem("S2", 200, 50, 40, 40);
    auto* s3 = new ThalesElementItem("S3", 400, 50, 40, 40);
    
    m_scene->addItem(s1);
    m_scene->addItem(s2);
    m_scene->addItem(s3);
    m_elements << s1 << s2 << s3;
    
    for (auto* el : m_elements) {
        connect(el, &ThalesElementItem::elementClicked, this, &ThalesWindow::onElementClicked);
    }
}

ThalesWindow::~ThalesWindow() = default;

void ThalesWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Top bar - Input and KOM
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_inputLine = new QLineEdit();
    m_komLine = new QLabel("KOM: System gotowy");
    m_komLine->setStyleSheet("QLabel { background-color : black; color : green; padding: 5px; font-weight: bold; }");
    topLayout->addWidget(new QLabel("WE:"));
    topLayout->addWidget(m_inputLine);
    topLayout->addWidget(m_komLine);
    
    // Commands layout
    QHBoxLayout* cmdLayout = new QHBoxLayout();
    m_btnPOC = new QPushButton("POC");
    m_btnMAN = new QPushButton("MAN");
    m_btnSZ = new QPushButton("SZ");
    m_btnDPZ = new QPushButton("DPZ");
    m_btnZW = new QPushButton("ZW");
    m_btnP = new QPushButton("P (Zatwierdz)");
    m_btnSPEC = new QPushButton("SPEC");
    m_btnSPEC->setStyleSheet("QPushButton { background-color: gray; }");
    
    cmdLayout->addWidget(m_btnPOC);
    cmdLayout->addWidget(m_btnMAN);
    cmdLayout->addWidget(m_btnSZ);
    cmdLayout->addWidget(m_btnDPZ);
    cmdLayout->addWidget(m_btnZW);
    cmdLayout->addStretch();
    cmdLayout->addWidget(m_btnSPEC);
    cmdLayout->addWidget(m_btnP);
    
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-50, -50, 800, 300);
    m_view = new QGraphicsView(m_scene);
    
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(cmdLayout);
    mainLayout->addWidget(m_view);
    
    resize(800, 600);
    setWindowTitle("Thales ESTW L90 5 (ML8) Simulator");
}

void ThalesWindow::onCommandButtonClicked(const QString& cmd) {
    if (m_state != State::IDLE) return;
    
    if (!m_inputBuffer.endsWith(", ") && !m_inputBuffer.isEmpty()) {
        m_inputBuffer += " ";
    }
    m_inputBuffer += cmd;
    m_inputLine->setText(m_inputBuffer);
}

void ThalesWindow::onElementClicked(const QString& id) {
    if (m_state != State::IDLE) return;
    
    // Zdarzenia z QGraphicsItem na klikniecie mysza beda jedynie doklejac identyfikatory
    if (!m_inputBuffer.isEmpty() && !m_inputBuffer.endsWith(", ")) {
        m_inputBuffer += ", ";
    }
    m_inputBuffer += id + ", ";
    m_inputLine->setText(m_inputBuffer);
    
    // Highlight logic
    for (auto* el : m_elements) {
        if (el->getId() == id) {
            el->setHighlight(true, Qt::yellow);
        }
    }
}

void ThalesWindow::onEnterPressed() {
    if (m_state != State::IDLE) return;
    m_inputBuffer = m_inputLine->text();
    ParseCommand(m_inputBuffer);
}

void ThalesWindow::ParseCommand(const QString& cmdStr) {
    QString cmd = cmdStr.trimmed();
    
    // Check if it is a special command requiring authorization
    if (cmd.contains("DPZ") || cmd.contains("ZW")) {
        // KROK 1: Inicjalizacja komendy (SPEC)
        for (auto* el : m_elements) {
            if (cmd.contains(el->getId())) {
                el->setHighlight(true, QColor(255, 165, 0)); // Pomaranczowy
            }
        }
        m_komLine->setText("KOM: Komenda niebezpieczna. Wymagana autoryzacja SPEC.");
        m_pendingSpecCommand = cmd;
        m_state = State::WAITING_FOR_SPEC;
        
        // KROK 2: Przygotowanie Timer'a
        m_btnSPEC->setStyleSheet("QPushButton { background-color: red; color: white; font-weight: bold; }");
        m_specTimer->start(25000); // 25 sekund
    } else {
        processCommand(cmd);
    }
}

void ThalesWindow::processCommand(const QString& cmd) {
    // Normal command execution (KROK 4)
    m_komLine->setText("KOM: Polecenie " + cmd + " zaakceptowane");
    
    if (cmd.contains("POC")) {
        // Rysowanie trasy (kolor zielony dla POC)
        for (auto* el : m_elements) {
            if (cmd.contains(el->getId())) {
                el->setPathColor(Qt::green);
                el->setHighlight(false);
            }
        }
    } else if (cmd.contains("MAN")) {
        // Rysowanie trasy (kolor zolty dla MAN)
        for (auto* el : m_elements) {
            if (cmd.contains(el->getId())) {
                el->setPathColor(Qt::yellow);
                el->setHighlight(false);
            }
        }
    }
    
    resetState();
}

void ThalesWindow::onSpecButtonClicked() {
    if (m_state == State::WAITING_FOR_SPEC) {
        m_specTimer->stop();
        executeSpecCommand();
    }
}

void ThalesWindow::executeSpecCommand() {
    // KROK 3: Zatwierdzenie autoryzacji
    m_komLine->setText("KOM: Wykonano polecenie specjalne: " + m_pendingSpecCommand);
    
    // Znika pomaranczowe podswietlenie, przerysowuje element
    for (auto* el : m_elements) {
        if (m_pendingSpecCommand.contains(el->getId())) {
            el->setPathColor(Qt::magenta); // Faktycznie wyslana komenda
            el->setHighlight(false);
        }
    }
    
    m_btnSPEC->setStyleSheet("QPushButton { background-color: gray; }");
    resetState();
}

void ThalesWindow::onSpecTimeout() {
    // Brak dzialania (Timeout)
    m_komLine->setText("KOM: Odrzucono. Timeout autoryzacji.");
    m_btnSPEC->setStyleSheet("QPushButton { background-color: gray; }");
    
    for (auto* el : m_elements) {
        el->setHighlight(false);
    }
    
    resetState();
}

void ThalesWindow::resetState() {
    m_inputBuffer.clear();
    m_inputLine->clear();
    m_pendingSpecCommand.clear();
    m_state = State::IDLE;
}
