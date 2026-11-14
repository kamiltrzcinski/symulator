#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QString>
#include <QColor>

class ThalesHeaderGraphic;

class ThalesBrowserWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ThalesBrowserWindow(QWidget* parent = nullptr);

private:
    QGraphicsView* m_view;
    QGraphicsScene* m_scene;
    ThalesHeaderGraphic* m_headerGraphic{nullptr};

    void setupBrowser();
    void addSectionHeader(const QString& title, qreal x, qreal y);
};
