#include "ui/thales/thales_main_window.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QDebug>

namespace symulator::client::ui::thales {

ThalesMainWindow::ThalesMainWindow(QWidget* parent)
    : QMainWindow(parent),
      awaitingSpec_(false)
{
    setupUi();

    specTimer_ = new QTimer(this);
    specTimer_->setSingleShot(true);
    connect(specTimer_, &QTimer::timeout, this, &ThalesMainWindow::handleSpecTimeout);
}

ThalesMainWindow::~ThalesMainWindow() = default;

void ThalesMainWindow::setupUi() {
    this->setWindowTitle("Thales ESTW L90 5 - Command 900");
    this->resize(1280, 800);
    this->setStyleSheet("QMainWindow { background-color: #1E1E1E; color: #FFFFFF; }");

    auto* centralWidget = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralWidget);

    // Top Bar (Command Input & Status)
    auto* topLayout = new QHBoxLayout();
    
    inputLine_ = new QLineEdit(this);
    inputLine_->setPlaceholderText("Linia danych wejściowych...");
    inputLine_->setStyleSheet("QLineEdit { background-color: #2D2D2D; color: white; border: 1px solid gray; padding: 5px; font-size: 16px; }");
    
    statusLine_ = new QLabel("OK", this);
    statusLine_->setStyleSheet("QLabel { color: #00FF00; font-weight: bold; padding: 5px; }");

    btnProcess_ = new QPushButton("P", this);
    btnProcess_->setFixedSize(40, 40);
    btnProcess_->setStyleSheet("QPushButton { background-color: #0055A4; color: white; font-weight: bold; border-radius: 5px; }");
    connect(btnProcess_, &QPushButton::clicked, this, &ThalesMainWindow::handleProcessCommand);

    btnSpec_ = new QPushButton("SPEC", this);
    btnSpec_->setFixedSize(60, 40);
    btnSpec_->setStyleSheet("QPushButton { background-color: gray; color: white; font-weight: bold; border-radius: 5px; }");
    btnSpec_->setEnabled(false);
    connect(btnSpec_, &QPushButton::clicked, this, &ThalesMainWindow::handleSpecCommand);

    topLayout->addWidget(inputLine_);
    topLayout->addWidget(statusLine_);
    topLayout->addWidget(btnProcess_);
    topLayout->addWidget(btnSpec_);

    mainLayout->addLayout(topLayout);

    // Map View
    mapScene_ = new QGraphicsScene(this);
    mapScene_->setBackgroundBrush(QColor("#000000")); // Thales uses black background for tracking
    
    mapView_ = new QGraphicsView(mapScene_, this);
    mapView_->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(mapView_);

    this->setCentralWidget(centralWidget);
}

void ThalesMainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        handleProcessCommand();
    } else if (event->key() == Qt::Key_A && (event->modifiers() & Qt::ControlModifier)) {
        if (awaitingSpec_) {
            handleSpecCommand();
        }
    } else if (event->key() == Qt::Key_Escape) {
        inputLine_->clear();
        statusLine_->setText("Anulowano wprowadzanie");
        statusLine_->setStyleSheet("QLabel { color: orange; }");
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void ThalesMainWindow::handleProcessCommand() {
    QString cmd = inputLine_->text().trimmed().toUpper();
    if (cmd.isEmpty()) return;

    // Simulate basic parsing logic
    if (cmd.contains("DPZ") || cmd.contains("ZEROLO") || cmd.contains("KSR")) {
        // Spec command detected
        pendingSpecCommand_ = cmd;
        awaitingSpec_ = true;
        btnSpec_->setEnabled(true);
        btnSpec_->setStyleSheet("QPushButton { background-color: red; color: white; font-weight: bold; border-radius: 5px; }");
        statusLine_->setText("Polecenie specjalne. Zatwierdź SPEC (20s).");
        statusLine_->setStyleSheet("QLabel { color: orange; }");
        specTimer_->start(20000); // 20 seconds
    } else {
        processCommand(cmd);
    }
}

void ThalesMainWindow::handleSpecCommand() {
    if (!awaitingSpec_) return;
    
    specTimer_->stop();
    awaitingSpec_ = false;
    btnSpec_->setEnabled(false);
    btnSpec_->setStyleSheet("QPushButton { background-color: gray; color: white; font-weight: bold; border-radius: 5px; }");
    
    processCommand(pendingSpecCommand_);
    pendingSpecCommand_.clear();
}

void ThalesMainWindow::handleSpecTimeout() {
    awaitingSpec_ = false;
    btnSpec_->setEnabled(false);
    btnSpec_->setStyleSheet("QPushButton { background-color: gray; color: white; font-weight: bold; border-radius: 5px; }");
    statusLine_->setText("Odrzucono (Timeout SPEC)");
    statusLine_->setStyleSheet("QLabel { color: red; }");
    pendingSpecCommand_.clear();
}

void ThalesMainWindow::processCommand(const QString& cmd) {
    qDebug() << "Executing Thales Command:" << cmd;
    inputLine_->clear();
    statusLine_->setText(QString("Polecenie %1 zaakceptowane").arg(cmd));
    statusLine_->setStyleSheet("QLabel { color: #00FF00; }");
}

} // namespace symulator::client::ui::thales
