#pragma once

#include <map>
#include <memory>
#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QString>
#include <QGraphicsSceneMouseEvent>
#include <QPen>

class ThalesElementItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    ThalesElementItem(const QString& id, qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent = nullptr);

    QString getId() const { return m_id; }
    void setHighlight(bool active, const QColor& color = Qt::yellow);
    void setPathColor(const QColor& color);

signals:
    void elementClicked(const QString& id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_id;
    bool m_highlighted;
};

class ThalesWindow;

class ICommandHandler {
public:
    virtual ~ICommandHandler() = default;
    virtual bool requiresSpecAuth() const = 0;
    virtual void execute(ThalesWindow* window, const QString& cmd) = 0;
};

class ThalesWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ThalesWindow(QWidget* parent = nullptr);
    ~ThalesWindow() override;

    void setStatusText(const QString& text);
    void highlightElement(const QString& id, const QColor& color);
    void setElementColor(const QString& id, const QColor& color);

private slots:
    void onCommandButtonClicked(const QString& cmd);
    void onElementClicked(const QString& id);
    void onEnterPressed();
    void onSpecButtonClicked();
    void onSpecTimeout();
    void processCommand(const QString& cmd);

private:
    void setupUi();
    void ParseCommand(const QString& cmdStr);
    void executeSpecCommand();
    void resetState();
    void registerCommands();

    QString m_inputBuffer;
    
    // UI Elements
    QGraphicsView* m_view;
    QGraphicsScene* m_scene;
    QLineEdit* m_inputLine;
    QLabel* m_komLine;
    QPushButton* m_btnPOC;
    QPushButton* m_btnMAN;
    QPushButton* m_btnSZ;
    QPushButton* m_btnDPZ;
    QPushButton* m_btnZW;
    QPushButton* m_btnP;
    QPushButton* m_btnSPEC;

    QTimer* m_specTimer;
    enum class State {
        IDLE,
        WAITING_FOR_SPEC
    };
    State m_state;
    QString m_pendingSpecCommand;
    
    QList<ThalesElementItem*> m_elements;
    std::map<QString, std::unique_ptr<ICommandHandler>> m_commandHandlers;
};
