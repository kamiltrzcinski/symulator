#ifndef THALES_TESTER_HPP
#define THALES_TESTER_HPP

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QStringList>
#include "thales/thales_browser.hpp"

class ThalesTesterWindow : public QWidget {
    Q_OBJECT

public slots:
    void reloadLayout(const QString& path);

public:
    explicit ThalesTesterWindow(QWidget* parent = nullptr);
    ~ThalesTesterWindow() override;

private slots:
    void onCommandButtonClicked(const QString& cmd);
    void onElementClicked(const QString& id);
    void onEnterPressed();
    void onSpecTimeout();
    

private:
    void setupUi();
    void buildTrackLayout();
    void processCommand(const QString& cmd);
    void resetState();
    void executeSpecCommand();

    QGraphicsView* m_view;
    QGraphicsScene* m_scene;
    
    // Command box integration
    QString m_inputBuffer;
    QString m_pendingSpecCommand;
    
    enum class State { IDLE, WAITING_FOR_SPEC };
    State m_state;
    QTimer* m_specTimer;

    // References to specific graphics items
    
    
    
    ThalesButtonGraphic* m_btnP;
    
    void updateCommandBox();
    void showKom(const QString& msg, QColor color = QColor(0, 255, 0));
};

#endif // THALES_TESTER_HPP
