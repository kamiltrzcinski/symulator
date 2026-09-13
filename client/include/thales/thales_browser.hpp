#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include <QFontMetrics>
#include <QString>
#include <QColor>

// ============================================================================
// 1. Przycisk Pulpitu Thales (wektorowo rysowany kodem z cieniami 3D)
// ============================================================================
class ThalesButtonGraphic : public QGraphicsItem {
public:
    QString id() const { return m_id; }
    void setId(const QString& id) { m_id = id; }
    QString text() const { return m_text; }
    void setText(const QString& t) { m_text = t; update(); }
    enum Style {
        BlueOT,      // Wypełnienie granatowe, tekst jasnoniebieski (np. OTSKI1)
        GraySystem,  // Wypełnienie szare, tekst jasnoszary (np. LOFF, HMI)
        RedPzb       // Wypełnienie bordowe, tekst czerwony (np. PZB)
    };

    ThalesButtonGraphic(const QString& text, Style style = BlueOT, bool hasArrow = false, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_text;
    Style m_style;
    bool m_hasArrow;
    bool m_pressed{false};
    QString m_id;
};

// ============================================================================
// 2. Odcinek Torowy (Track)
// ============================================================================
class ThalesTrackGraphic : public QGraphicsItem {
public:
    QString sectionId() const { return m_sectionId; }
    void setSectionId(const QString& id) { m_sectionId = id; update(); }
    qreal length() const { return m_length; }
    void setLength(qreal len) { m_length = len; prepareGeometryChange(); update(); }
    enum TrackState {
        Free,               // Wolny (jasnoszary #9B9B9B, 4px)
        Occupied,           // Zajęty (czerwony #FF0000, 4px)
        RouteLockedTrain,   // Utwierdzony pociągowy (zielony #00FF00, 4px)
        RouteLockedShunt,   // Utwierdzony manewrowy (żółty #FFFF00, 4px)
        MagentaRelease,     // Zwalnianie z opóźnieniem (magenta #FF00FF, 4px)
        FaultBlinking,      // Usterka licznika osi (czerwono-biały migający)
        PreReset            // Reset wstępny / oczekiwanie na pociąg (ciemnoczerwony #8B0000)
    };

    enum Termination {
        None,
        BufferStopLeft,     // Kozioł oporowy po lewej
        BufferStopRight     // Kozioł oporowy po prawej
    };

    ThalesTrackGraphic(qreal length, TrackState state = Free, Termination term = None, const QString& trackNum = QString(), QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setTrainNum(const QString& num) { m_trainNum = num; update(); }
    QString trainNum() const { return m_trainNum; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    qreal m_length;
    QString m_sectionId;
    TrackState m_state;
    Termination m_termination;
    QString m_trackNum;
    QString m_trainNum;
    bool m_selected{false};
};

// ============================================================================
// 3. Sygnalizatory pociągowe i manewrowe w linii toru (Semafory << >> i tarcze < >)
// ============================================================================
class ThalesSignalGraphic : public QGraphicsItem {
public:
    enum SignalType {
        TrainLeft,          // Tylko pociągowy (semafor) w lewo <|
        TrainRight,         // Tylko pociągowy (semafor) w prawo |>
        ShuntLeft,          // Tylko manewrowy (tarcza) w lewo <
        ShuntRight,         // Tylko manewrowy (tarcza) w prawo >
        TrainAndShuntLeft,  // Pociągowy + manewrowy w lewo <<|
        TrainAndShuntRight  // Pociągowy + manewrowy w prawo |>>
    };

    enum SignalState {
        Stop,               // Sygnał Stój (kolor szary toru #9B9B9B)
        ProceedTrain,       // Przebieg pociągowy (Zielony #00FF00)
        ProceedShunt,       // Przebieg manewrowy (Żółty #FFFF00)
        Substitute,         // Sygnał zastępczy Sz (migający biały – priorytet nad Stój)
        SignalStopped       // Zastopowanie sygnalizatora (magenta #FF00FF)
    };

    QString name() const { return m_name; }
    void setName(const QString& n) { m_name = n; update(); }
    SignalType signalType() const { return m_type; }
    void setSignalType(SignalType t) { m_type = t; prepareGeometryChange(); update(); }

    ThalesSignalGraphic(const QString& name, SignalType type, SignalState state = Stop, bool labelAbove = true, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_name;
    SignalType m_type;
    SignalState m_state;
    bool m_labelAbove;
    bool m_selected{false};
};

// ============================================================================
// 4. Punkt Końcowy Przebiegu Manewrowego (PKPM - żółty trójkąt, większy)
// ============================================================================
class ThalesPkpmGraphic : public QGraphicsItem {
public:
    enum Direction {
        Left,   // Trójkąt w lewo ◀
        Right   // Trójkąt w prawo ▶
    };

    ThalesPkpmGraphic(Direction dir, const QString& trackNum = "", QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Direction m_dir;
    QString m_trackNum;
    bool m_selected{false};
};

// ============================================================================
// 5. Wskaźnik Końca Elektryfikacji (Czerwona skośna strzałka ↙)
// ============================================================================
class ThalesEndCatenaGraphic : public QGraphicsItem {
public:
    ThalesEndCatenaGraphic(const QString& trackNum = "", QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_trackNum;
    bool m_selected{false};
};

// ============================================================================
// 6. Rozjazd (Switch - stromy kąt ok. 70° z przerwą izolowaną)
// ============================================================================
class ThalesSwitchGraphic : public QGraphicsItem {
public:
    enum BranchDir {
        BranchUpRight,
        BranchDownRight,
        BranchUpLeft,
        BranchDownLeft
    };

    enum SwitchState {
        SwitchNormal,    // Normalny (szary wg zajętości)
        SwitchStopped,   // Zastopowanie / zamknięcie indywidualne (magenta #FF00FF)
        SwitchNoControl, // Brak kontroli położenia (migający biały kwadrat u podstawy)
        SwitchDerailed   // Rozprucie zwrotnicy (migający czerwony)
    };
    
    QString name() const { return m_name; }
    void setName(const QString& n) { m_name = n; update(); }
    BranchDir branch() const { return m_dir; }
    void setBranch(BranchDir d) { m_dir = d; prepareGeometryChange(); update(); }
    SwitchState switchState() const { return m_switchState; }
    void setSwitchState(SwitchState s) { m_switchState = s; update(); }
    bool isDiverging() const { return m_diverging; }
    void setDiverging(bool d) { m_diverging = d; update(); }
    QPointF branchEndpoint() const;

    ThalesSwitchGraphic(const QString& name, BranchDir dir, bool divergingOccupied = false, SwitchState state = SwitchNormal, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_name;
    BranchDir m_dir;
    bool m_divergingOccupied;
    SwitchState m_switchState;
    bool m_diverging{false};
    bool m_selected{false};
};

// ============================================================================
// 7. Obiekt Peronu (Platform - podwójna linia z tekstem w środku)
// ============================================================================
class ThalesPlatformGraphic : public QGraphicsItem {
public:
    ThalesPlatformGraphic(const QString& text, qreal width = 85, qreal height = 20, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QString m_text;
    qreal m_width;
    qreal m_height;
};

class ThalesCommandBoxGraphic : public QGraphicsItem {
public:
    ThalesCommandBoxGraphic(qreal width = 450, qreal height = 70, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setWeText(const QString& text) { m_weText = text; update(); }
    void setKomText(const QString& text) { m_komText = text; update(); }
    void setXxText(const QString& text) { m_xxText = text; update(); }

private:
    qreal m_width;
    qreal m_height;
    QString m_weText;
    QString m_komText;
    QString m_xxText;
};

// ============================================================================
// 9. Nagłówek systemowy (Logo THALES RSS, zegar, wskaźniki P / R G B)
// ============================================================================
class ThalesHeaderGraphic : public QGraphicsItem {
public:
    ThalesHeaderGraphic(QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

// ============================================================================
// 10. Etykieta Tekstowa (Label)
// ============================================================================
class ThalesLabelGraphic : public QGraphicsItem {
public:
    QString text() const { return m_text; }
    ThalesLabelGraphic(const QString& text, QColor color = QColor(155, 155, 155), bool isHeader = false, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setText(const QString& text) { m_text = text; update(); }
    void setPen(QColor color) { m_color = color; update(); }

private:
    QString m_text;
    QColor m_color;
    bool m_isHeader;
};

// ============================================================================
// 11. Blokada Liniowa
// ============================================================================
class ThalesLineBlockGraphic : public QGraphicsItem {
public:
    QString label() const { return m_label; }
    void setLabel(const QString& l) { m_label = l; update(); }
    enum State {
        Neutral,              // Brak nadanego kierunku (szare strzałki w obie strony)
        Sending,              // Wyjazd dozwolony / kierunek nadany (żółta strzałka w prawo)
        Receiving,            // Przyjazd dozwolony / kierunek odebrany (żółta strzałka w lewo)
        PermissionRequested,  // Żądanie pozwolenia (migająca żółta strzałka prążkowana)
        EmergencyChange       // Awaryjna zmiana kierunku (migająca czerwona)
    };
    ThalesLineBlockGraphic(State state = Neutral, const QString& label = "", QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    State m_state;
    QString m_label;
    bool m_selected{false};
};

// ============================================================================
// 12. Symbol Nastawni
// ============================================================================
class ThalesSignalBoxGraphic : public QGraphicsItem {
public:
    QString name() const { return m_label; }
    void setName(const QString& n) { m_label = n; update(); }
    ThalesSignalBoxGraphic(const QString& label, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_label;
    bool m_selected{false};
};

// ============================================================================
// 13. Wykolejnica (Derailment Device)
// ============================================================================
class ThalesDerailGraphic : public QGraphicsItem {
public:
    enum DerailState {
        Placed,     // Nałożona (zrzucająca) – gruba pionowa kreska na torze, czerwona
        Clear,      // Zdjęta (przejezdna) – dwie cienkie równoległe kreski
        Stopped,    // Zastopowanie wykolejnicy (magenta #FF00FF)
        NoControl   // Brak kontroli położenia (migający biały element)
    };

    enum Direction { Left, Right };

    ThalesDerailGraphic(const QString& name, Direction dir = Right, DerailState state = Clear, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    DerailState derailState() const { return m_state; }
    void setDerailState(DerailState s) { m_state = s; update(); }
    bool isOccupied() const { return m_occupied; }
    void setOccupied(bool o) { m_occupied = o; update(); }
    QString name() const { return m_name; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_name;
    Direction m_dir;
    DerailState m_state;
    bool m_occupied{false};
    bool m_selected{false};
};

// ============================================================================
// 14. Licznik Osi / Punkt Pomiarowy (Axle Counter)
// ============================================================================
class ThalesAxleCounterGraphic : public QGraphicsItem {
public:
    enum AxleState {
        OK,         // Gotowy / wolny (szary #9B9B9B)
        Occupied,   // Zajęty – pociąg w sekcji (czerwony #FF0000)
        Fault,      // Usterka urządzenia (czerwono-biały migający)
        AxlePreReset // Reset wstępny / oczekiwanie na pociąg (ciemnoczerwony #8B0000)
    };

    ThalesAxleCounterGraphic(const QString& sectionId, AxleState state = OK, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    AxleState axleState() const { return m_state; }
    void setAxleState(AxleState s) { m_state = s; update(); }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_sectionId;
    AxleState m_state;
    bool m_selected{false};
};

// ============================================================================
// Główne Okno Przeglądarki Obiektów Thales ML8
// ============================================================================
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
