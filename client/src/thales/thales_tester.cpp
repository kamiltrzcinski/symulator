#include "thales/thales_tester.hpp"
#include <QVBoxLayout>
#include <QDebug>
#include <QKeyEvent>
#include <QShortcut>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include "thales/topology_loader.hpp"
#include <QMessageBox>

ThalesTesterWindow::ThalesTesterWindow(QWidget* parent) : QWidget(parent), m_state(State::IDLE) {
    m_specTimer = new QTimer(this);
    m_specTimer->setSingleShot(true);
    connect(m_specTimer, &QTimer::timeout, this, &ThalesTesterWindow::onSpecTimeout);

    setupUi();
    buildTrackLayout();
}

ThalesTesterWindow::~ThalesTesterWindow() = default;

void ThalesTesterWindow::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush(QColor(40, 40, 40));

    m_view = new QGraphicsView(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    
    mainLayout->addWidget(m_view);

    connect(new QShortcut(QKeySequence(Qt::Key_Enter), this), &QShortcut::activated, this, &ThalesTesterWindow::onEnterPressed);
    connect(new QShortcut(QKeySequence(Qt::Key_Return), this), &QShortcut::activated, this, &ThalesTesterWindow::onEnterPressed);
}

void ThalesTesterWindow::buildTrackLayout() {
    if (!TopologyLoader::loadFromJson("layout.json", m_scene)) {
        qWarning() << "Failed to load layout.json!";
    }
}

void ThalesTesterWindow::updateCommandBox() {
    ThalesCommandBoxGraphic* cmdBox = nullptr;
    for (QGraphicsItem* item : m_scene->items()) {
        cmdBox = dynamic_cast<ThalesCommandBoxGraphic*>(item);
        if (cmdBox) break;
    }
    if (cmdBox) {
        cmdBox->setWeText(m_inputBuffer);
        cmdBox->setKomText(m_pendingSpecCommand);
    }
}

void ThalesTesterWindow::showKom(const QString& msg, QColor color) {
    ThalesCommandBoxGraphic* cmdBox = nullptr;
    for (QGraphicsItem* item : m_scene->items()) {
        cmdBox = dynamic_cast<ThalesCommandBoxGraphic*>(item);
        if (cmdBox) break;
    }
    if (cmdBox) {
        cmdBox->setKomText("KOM: " + msg);
    }
}

void ThalesTesterWindow::onCommandButtonClicked(const QString& cmd) {
    if (m_state != State::IDLE) return;
    if (!m_inputBuffer.endsWith(", ") && !m_inputBuffer.isEmpty()) m_inputBuffer += " ";
    m_inputBuffer += cmd;
    updateCommandBox();
}

void ThalesTesterWindow::onElementClicked(const QString& id) {
    if (m_state != State::IDLE) return;
    if (!m_inputBuffer.endsWith(", ") && !m_inputBuffer.isEmpty()) m_inputBuffer += ", ";
    m_inputBuffer += id;
    updateCommandBox();
}

void ThalesTesterWindow::onEnterPressed() {
    if (m_state == State::IDLE) {
        processCommand(m_inputBuffer);
    } else if (m_state == State::WAITING_FOR_SPEC) {
        executeSpecCommand();
    }
}

void ThalesTesterWindow::processCommand(const QString& cmd) {
    m_inputBuffer.clear();
    updateCommandBox();
}

void ThalesTesterWindow::resetState() {
    m_state = State::IDLE;
    m_inputBuffer.clear();
    m_pendingSpecCommand.clear();
    updateCommandBox();
}

void ThalesTesterWindow::executeSpecCommand() {
    showKom("Wykonano przebieg", QColor(0, 255, 0));
    resetState();
}

void ThalesTesterWindow::onSpecTimeout() {
    resetState();
    showKom("Brak autoryzacji", QColor(255, 0, 0));
}

void ThalesTesterWindow::reloadLayout(const QString& path) {
    m_scene->clear();
    TopologyLoader::loadFromJson(path, m_scene);
    updateCommandBox();
}
