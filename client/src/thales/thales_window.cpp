#include "thales/thales_window.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QShortcut>
#include <QDebug>

class PocCommandHandler : public ICommandHandler {
public:
    bool requiresSpecAuth() const override { return false; }
    void execute(ThalesWindow* window, const QString& cmd) override {
        window->setStatusText("KOM: Polecenie " + cmd + " zaakceptowane");
        // Extract elements from cmd (e.g. "S1, POC")
        QStringList parts = cmd.split(",");
        for (const QString& part : parts) {
            QString id = part.trimmed();
            if (id != "POC") {
                window->setElementColor(id, Qt::green);
                window->highlightElement(id, Qt::transparent); // remove highlight
            }
        }
    }
};

class ManCommandHandler : public ICommandHandler {
public:
    bool requiresSpecAuth() const override { return false; }
    void execute(ThalesWindow* window, const QString& cmd) override {
        window->setStatusText("KOM: Polecenie " + cmd + " zaakceptowane");
        QStringList parts = cmd.split(",");
        for (const QString& part : parts) {
            QString id = part.trimmed();
            if (id != "MAN") {
                window->setElementColor(id, Qt::yellow);
                window->highlightElement(id, Qt::transparent);
            }
        }
    }
};

class DpzCommandHandler : public ICommandHandler {
public:
    bool requiresSpecAuth() const override { return true; }
    void execute(ThalesWindow* window, const QString& cmd) override {
        window->setStatusText("KOM: Wykonano polecenie specjalne: " + cmd);
        QStringList parts = cmd.split(",");
        for (const QString& part : parts) {
            QString id = part.trimmed();
            if (id != "DPZ") {
                window->setElementColor(id, Qt::magenta);
                window->highlightElement(id, Qt::transparent);
            }
        }
    }
};

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
    
    registerCommands();

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

void ThalesWindow::registerCommands() {
    m_commandHandlers["POC"] = std::make_unique<PocCommandHandler>();
    m_commandHandlers["MAN"] = std::make_unique<ManCommandHandler>();
    m_commandHandlers["DPZ"] = std::make_unique<DpzCommandHandler>();
    // Other commands would be registered here...
}

void ThalesWindow::setStatusText(const QString& text) {
    m_komLine->setText(text);
}

void ThalesWindow::highlightElement(const QString& id, const QColor& color) {
    for (auto* el : m_elements) {
        if (el->getId() == id) {
            el->setHighlight(color != Qt::transparent, color);
        }
    }
}

void ThalesWindow::setElementColor(const QString& id, const QColor& color) {
    for (auto* el : m_elements) {
        if (el->getId() == id) {
            el->setPathColor(color);
        }
    }
}

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
    
    if (!m_inputBuffer.isEmpty() && !m_inputBuffer.endsWith(", ")) {
        m_inputBuffer += ", ";
    }
    m_inputBuffer += id + ", ";
    m_inputLine->setText(m_inputBuffer);
    
    highlightElement(id, Qt::yellow);
}

void ThalesWindow::onEnterPressed() {
    if (m_state != State::IDLE) return;
    m_inputBuffer = m_inputLine->text();
    ParseCommand(m_inputBuffer);
}

void ThalesWindow::ParseCommand(const QString& cmdStr) {
    QString cmd = cmdStr.trimmed();
    
    // Find the command token in the string (e.g. "POC", "DPZ")
    QString commandName;
    for (auto it = m_commandHandlers.begin(); it != m_commandHandlers.end(); ++it) {
        if (cmd.contains(it->first)) {
            commandName = it->first;
            break;
        }
    }

    if (commandName.isEmpty()) {
        setStatusText("KOM: Nieznana komenda.");
        return;
    }

    ICommandHandler* handler = m_commandHandlers[commandName].get();

    if (handler->requiresSpecAuth()) {
        // KROK 1: Inicjalizacja komendy (SPEC)
        QStringList parts = cmd.split(",");
        for (const QString& part : parts) {
            QString id = part.trimmed();
            if (id != commandName) {
                highlightElement(id, QColor(255, 165, 0)); // Pomaranczowy
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
    for (auto it = m_commandHandlers.begin(); it != m_commandHandlers.end(); ++it) {
        if (cmd.contains(it->first)) {
            it->second->execute(this, cmd);
            break;
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
    for (auto it = m_commandHandlers.begin(); it != m_commandHandlers.end(); ++it) {
        if (m_pendingSpecCommand.contains(it->first)) {
            it->second->execute(this, m_pendingSpecCommand);
            break;
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
