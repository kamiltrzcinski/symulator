#include <cmath>
#include <QButtonGroup>
#include "thales/thales_editor.hpp"
#include "thales/thales_browser.hpp"
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QSplitter>

class ThalesEditorScene : public QGraphicsScene {
public:
    ThalesEditorScene(QObject* parent = nullptr) : QGraphicsScene(parent), m_editor(nullptr) {}
    ThalesEditorWindow* m_editor;
    void setEditor(ThalesEditorWindow* ed) { m_editor = ed; }
    
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

protected:
    
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        QGraphicsScene::mouseReleaseEvent(event);
        if (m_editor && m_editor->m_snapGrid) {
            for (auto item : selectedItems()) {
                qreal newX = qRound(item->x() / 10.0) * 10.0;
                qreal newY = qRound(item->y() / 10.0) * 10.0;
                item->setPos(newX, newY);
            }
        }
    }

    void drawBackground(QPainter* painter, const QRectF& rect) override {
        QGraphicsScene::drawBackground(painter, rect);
        if (m_editor && !m_editor->m_snapGrid) return; // Don't draw if disabled

        qreal left = int(rect.left()) - (int(rect.left()) % 10);
        qreal top = int(rect.top()) - (int(rect.top()) % 10);
        QVarLengthArray<QLineF, 100> lines;
        for (qreal x = left; x < rect.right(); x += 10)
            lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += 10)
            lines.append(QLineF(rect.left(), y, rect.right(), y));
        QPen pen(QColor(60, 60, 60));
        pen.setWidth(0);
        painter->setPen(pen);
        painter->drawLines(lines.data(), lines.size());
    }
};

ThalesEditorWindow::ThalesEditorWindow(QWidget* parent) : QWidget(parent), m_selectedItem(nullptr) {
    setupUi();
}

ThalesEditorWindow::~ThalesEditorWindow() = default;

void ThalesEditorWindow::setupUi() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(splitter);

    // 1. Palette
    QWidget* paletteWidget = new QWidget();
    QVBoxLayout* paletteLayout = new QVBoxLayout(paletteWidget);
    paletteWidget->setFixedWidth(150);
    
    QPushButton* btnSave = new QPushButton("Eksportuj do Testera");
    btnSave->setMinimumHeight(40);
    btnSave->setStyleSheet("background-color: #2e8b57; color: white; font-weight: bold;");
    paletteLayout->addWidget(btnSave);
    paletteLayout->addSpacing(20);
    
    
    QPushButton* btnSelect = new QPushButton("Tryb: Wybierz");
    btnSelect->setCheckable(true);
    btnSelect->setChecked(true);
    QPushButton* btnKnife = new QPushButton("Tryb: Nóż");
    btnKnife->setCheckable(true);
    QPushButton* btnMerge = new QPushButton("Tryb: Scal");
    btnMerge->setCheckable(true);
    
    QButtonGroup* modeGroup = new QButtonGroup(this);
    modeGroup->addButton(btnSelect);
    modeGroup->addButton(btnKnife);
    modeGroup->addButton(btnMerge);
    
    paletteLayout->addWidget(btnSelect);
    paletteLayout->addWidget(btnKnife);
    paletteLayout->addWidget(btnMerge);
    paletteLayout->addSpacing(10);

    QCheckBox* chkGrid = new QCheckBox("Siatka i Przyciąganie");
    chkGrid->setChecked(true);
    connect(chkGrid, &QCheckBox::toggled, this, &ThalesEditorWindow::toggleGrid);
    paletteLayout->addWidget(chkGrid);
    paletteLayout->addSpacing(10);


    QPushButton* btnTrk = new QPushButton("Dodaj Tor");
    QPushButton* btnSig = new QPushButton("Dodaj Semafor");
    QPushButton* btnSw = new QPushButton("Dodaj Rozjazd");
    QPushButton* btnLb = new QPushButton("Dodaj Blokade");
    QPushButton* btnPkpm = new QPushButton("Dodaj PKPM");
    QPushButton* btnEc = new QPushButton("Dodaj Koniec Trakcji");
    QPushButton* btnPl = new QPushButton("Dodaj Peron");

    paletteLayout->addWidget(btnTrk);
    paletteLayout->addWidget(btnSig);
    paletteLayout->addWidget(btnSw);
    paletteLayout->addWidget(btnLb);
    paletteLayout->addWidget(btnPkpm);
    paletteLayout->addWidget(btnEc);
    paletteLayout->addWidget(btnPl);
    QPushButton* btnBtn = new QPushButton("Dodaj Przycisk");
    QPushButton* btnCmd = new QPushButton("Dodaj Okno Komend");
    QPushButton* btnHdr = new QPushButton("Dodaj Nagłówek");
    QPushButton* btnLbl = new QPushButton("Dodaj Etykietę");
    QPushButton* btnBox = new QPushButton("Dodaj Nastawnię");
    
    paletteLayout->addWidget(btnBtn);
    paletteLayout->addWidget(btnCmd);
    paletteLayout->addWidget(btnHdr);
    paletteLayout->addWidget(btnLbl);
    paletteLayout->addWidget(btnBox);

    paletteLayout->addStretch();
    
    // 2. View
    ThalesEditorScene* edScene = new ThalesEditorScene(this);
    edScene->setEditor(this);
    m_scene = edScene;
    m_scene->setBackgroundBrush(QColor(40, 40, 40));
    m_view = new QGraphicsView(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);

    // 3. Inspector
    QWidget* inspectorWidget = new QWidget();
    QVBoxLayout* inspMainLayout = new QVBoxLayout(inspectorWidget);
    inspectorWidget->setFixedWidth(250);
    
    m_inspectorGroup = new QGroupBox("Wlasciwosci");
    m_inspectorLayout = new QFormLayout(m_inspectorGroup);
    inspMainLayout->addWidget(m_inspectorGroup);
    inspMainLayout->addStretch();

    splitter->addWidget(paletteWidget);
    splitter->addWidget(m_view);
    splitter->addWidget(inspectorWidget);
    splitter->setSizes({150, 800, 250});

    // Connections
    connect(btnSave, &QPushButton::clicked, this, &ThalesEditorWindow::onSaveClicked);
    connect(btnSelect, &QPushButton::clicked, this, &ThalesEditorWindow::setModeSelect);
    connect(btnKnife, &QPushButton::clicked, this, &ThalesEditorWindow::setModeKnife);
    connect(btnMerge, &QPushButton::clicked, this, &ThalesEditorWindow::setModeMerge);
    connect(btnTrk, &QPushButton::clicked, this, &ThalesEditorWindow::spawnTrack);
    connect(btnSig, &QPushButton::clicked, this, &ThalesEditorWindow::spawnSignal);
    connect(btnSw, &QPushButton::clicked, this, &ThalesEditorWindow::spawnSwitch);
    connect(btnLb, &QPushButton::clicked, this, &ThalesEditorWindow::spawnLineBlock);
    connect(btnPkpm, &QPushButton::clicked, this, &ThalesEditorWindow::spawnPkpm);
    connect(btnEc, &QPushButton::clicked, this, &ThalesEditorWindow::spawnEndCatena);
    connect(btnPl, &QPushButton::clicked, this, &ThalesEditorWindow::spawnPlatform);
    connect(btnBtn, &QPushButton::clicked, this, &ThalesEditorWindow::spawnButton);
    connect(btnCmd, &QPushButton::clicked, this, &ThalesEditorWindow::spawnCommandBox);
    connect(btnHdr, &QPushButton::clicked, this, &ThalesEditorWindow::spawnHeader);
    connect(btnLbl, &QPushButton::clicked, this, &ThalesEditorWindow::spawnLabel);
    connect(btnBox, &QPushButton::clicked, this, &ThalesEditorWindow::spawnSignalBox);

    
    connect(m_scene, &QGraphicsScene::selectionChanged, this, &ThalesEditorWindow::onSelectionChanged);
}

void ThalesEditorWindow::clearInspector() {
    QLayoutItem* child;
    while ((child = m_inspectorLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }
}

void ThalesEditorWindow::onSelectionChanged() {
    clearInspector();
    m_selectedItem = nullptr;
    
    auto items = m_scene->selectedItems();
    if (items.isEmpty()) return;
    
    
    if (items.size() == 2) {
        auto sw1 = dynamic_cast<ThalesSwitchGraphic*>(items[0]);
        auto sw2 = dynamic_cast<ThalesSwitchGraphic*>(items[1]);
        if (sw1 && sw2) {
            QPushButton* btnConn = new QPushButton("Połącz rozjazdy (utwórz przejście)");
            connect(btnConn, &QPushButton::clicked, this, &ThalesEditorWindow::onConnectSwitchesClicked);
            m_inspectorLayout->addRow(btnConn);
            return;
        }
    }
    m_selectedItem = items.first();

    
    // Common properties
    m_spinRotation = new QDoubleSpinBox();
    m_spinRotation->setRange(0, 360);
    m_spinRotation->setSingleStep(45);
    m_spinRotation->setValue(m_selectedItem->rotation());
    connect(m_spinRotation, &QDoubleSpinBox::valueChanged, this, &ThalesEditorWindow::onPropertyChanged);
    m_inspectorLayout->addRow("Rotacja:", m_spinRotation);
    
    // Specific properties
    if (auto trk = dynamic_cast<ThalesTrackGraphic*>(m_selectedItem)) {
        QDoubleSpinBox* spinLen = new QDoubleSpinBox();
        spinLen->setRange(10, 1000);
        spinLen->setValue(trk->length());
        spinLen->setObjectName("prop_len");
        connect(spinLen, &QDoubleSpinBox::valueChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Dlugosc:", spinLen);
    } else if (auto sig = dynamic_cast<ThalesSignalGraphic*>(m_selectedItem)) {
        QLineEdit* editName = new QLineEdit(sig->name());
        editName->setObjectName("prop_name");
        connect(editName, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Nazwa:", editName);
        
        QComboBox* cmbType = new QComboBox();
        cmbType->addItems({"TrainLeft", "TrainRight", "ShuntLeft", "ShuntRight", "TrainAndShuntLeft", "TrainAndShuntRight"});
        cmbType->setCurrentIndex(sig->signalType());
        cmbType->setObjectName("prop_sigType");
        connect(cmbType, &QComboBox::currentIndexChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Typ:", cmbType);
    } else if (auto sw = dynamic_cast<ThalesSwitchGraphic*>(m_selectedItem)) {
        QLineEdit* editName = new QLineEdit(sw->name());
        editName->setObjectName("prop_name");
        connect(editName, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Nazwa:", editName);
        
        QComboBox* cmbDir = new QComboBox();
        cmbDir->addItems({"BranchUpRight", "BranchDownRight", "BranchUpLeft", "BranchDownLeft"});
        cmbDir->setCurrentIndex(sw->branch());
        cmbDir->setObjectName("prop_swDir");
        connect(cmbDir, &QComboBox::currentIndexChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Kierunek:", cmbDir);

    } else if (auto btn = dynamic_cast<ThalesButtonGraphic*>(m_selectedItem)) {
        QLineEdit* editId = new QLineEdit(btn->id());
        editId->setObjectName("prop_id");
        connect(editId, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("ID:", editId);
        QLineEdit* editText = new QLineEdit(btn->text());
        editText->setObjectName("prop_text");
        connect(editText, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Tekst:", editText);
    } else if (auto lbl = dynamic_cast<ThalesLabelGraphic*>(m_selectedItem)) {
        QLineEdit* editText = new QLineEdit(lbl->text());
        editText->setObjectName("prop_text");
        connect(editText, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Tekst:", editText);
    } else if (auto sbox = dynamic_cast<ThalesSignalBoxGraphic*>(m_selectedItem)) {
        QLineEdit* editText = new QLineEdit(sbox->name());
        editText->setObjectName("prop_text");
        connect(editText, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Nazwa:", editText);
    } else if (auto lb = dynamic_cast<ThalesLineBlockGraphic*>(m_selectedItem)) {
        QLineEdit* editName = new QLineEdit(lb->label());
        editName->setObjectName("prop_name");
        connect(editName, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Etykieta:", editName);
    }
}

void ThalesEditorWindow::onPropertyChanged() {
    if (!m_selectedItem) return;
    
    // Block signals so we don't infinitely recurse if properties affect others, not strictly needed but good practice
    
    m_selectedItem->setRotation(m_spinRotation->value());
    
    if (auto trk = dynamic_cast<ThalesTrackGraphic*>(m_selectedItem)) {
        auto spinLen = m_inspectorGroup->findChild<QDoubleSpinBox*>("prop_len");
        if (spinLen) trk->setLength(spinLen->value());
    } else if (auto sig = dynamic_cast<ThalesSignalGraphic*>(m_selectedItem)) {
        auto editName = m_inspectorGroup->findChild<QLineEdit*>("prop_name");
        if (editName) sig->setName(editName->text());
        auto cmbType = m_inspectorGroup->findChild<QComboBox*>("prop_sigType");
        if (cmbType) sig->setSignalType(static_cast<ThalesSignalGraphic::SignalType>(cmbType->currentIndex()));
    } else if (auto sw = dynamic_cast<ThalesSwitchGraphic*>(m_selectedItem)) {
        auto editName = m_inspectorGroup->findChild<QLineEdit*>("prop_name");
        if (editName) sw->setName(editName->text());
        auto cmbDir = m_inspectorGroup->findChild<QComboBox*>("prop_swDir");
        if (cmbDir) sw->setBranch(static_cast<ThalesSwitchGraphic::BranchDir>(cmbDir->currentIndex()));

    } else if (auto btn = dynamic_cast<ThalesButtonGraphic*>(m_selectedItem)) {
        QLineEdit* editId = new QLineEdit(btn->id());
        editId->setObjectName("prop_id");
        connect(editId, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("ID:", editId);
        QLineEdit* editText = new QLineEdit(btn->text());
        editText->setObjectName("prop_text");
        connect(editText, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Tekst:", editText);
    } else if (auto lbl = dynamic_cast<ThalesLabelGraphic*>(m_selectedItem)) {
        QLineEdit* editText = new QLineEdit(lbl->text());
        editText->setObjectName("prop_text");
        connect(editText, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Tekst:", editText);
    } else if (auto sbox = dynamic_cast<ThalesSignalBoxGraphic*>(m_selectedItem)) {
        QLineEdit* editText = new QLineEdit(sbox->name());
        editText->setObjectName("prop_text");
        connect(editText, &QLineEdit::textChanged, this, &ThalesEditorWindow::onPropertyChanged);
        m_inspectorLayout->addRow("Nazwa:", editText);
    } else if (auto lb = dynamic_cast<ThalesLineBlockGraphic*>(m_selectedItem)) {
        auto editName = m_inspectorGroup->findChild<QLineEdit*>("prop_name");
        if (editName) lb->setLabel(editName->text());
    }
}

void ThalesEditorWindow::spawnTrack() {
    auto trk = new ThalesTrackGraphic(100);
    trk->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(trk);
}

void ThalesEditorWindow::spawnSignal() {
    auto sig = new ThalesSignalGraphic("TmX", ThalesSignalGraphic::ShuntRight);
    sig->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(sig);
}

void ThalesEditorWindow::spawnSwitch() {
    auto sw = new ThalesSwitchGraphic("X", ThalesSwitchGraphic::BranchDownRight);
    sw->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(sw);
}

void ThalesEditorWindow::spawnLineBlock() {
    auto lb = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Neutral, "1L");
    lb->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(lb);
}

void ThalesEditorWindow::spawnPkpm() {
    auto pkpm = new ThalesPkpmGraphic(ThalesPkpmGraphic::Right);
    pkpm->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(pkpm);
}

void ThalesEditorWindow::spawnEndCatena() {
    auto ec = new ThalesEndCatenaGraphic();
    ec->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(ec);
}

void ThalesEditorWindow::spawnPlatform() {
    auto pl = new ThalesPlatformGraphic("Peron", 100, 20);
    pl->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(pl);
}


void ThalesEditorWindow::spawnButton() {
    auto b = new ThalesButtonGraphic("Przycisk");
    b->setId("B1");
    b->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(b);
}
void ThalesEditorWindow::spawnCommandBox() {
    auto c = new ThalesCommandBoxGraphic();
    c->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(c);
}
void ThalesEditorWindow::spawnHeader() {
    auto h = new ThalesHeaderGraphic();
    h->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(h);
}
void ThalesEditorWindow::spawnLabel() {
    auto l = new ThalesLabelGraphic("Etykieta", QColor(155, 155, 155), false);
    l->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(l);
}
void ThalesEditorWindow::spawnSignalBox() {
    auto b = new ThalesSignalBoxGraphic("Nastawnia");
    b->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(b);
}


void ThalesEditorWindow::onConnectSwitchesClicked() {
    auto items = m_scene->selectedItems();
    if (items.size() != 2) return;
    auto sw1 = dynamic_cast<ThalesSwitchGraphic*>(items[0]);
    auto sw2 = dynamic_cast<ThalesSwitchGraphic*>(items[1]);
    if (!sw1 || !sw2) return;

    QPointF p1 = sw1->branchEndpoint();
    QPointF p2 = sw2->branchEndpoint();

    qreal dx = p2.x() - p1.x();
    qreal dy = p2.y() - p1.y();
    qreal dist = std::sqrt(dx*dx + dy*dy);
    qreal angle = std::atan2(dy, dx) * 180.0 / 3.14159265358979323846;

    // Tworzymy dwa tory, zeby mozna bylo zrobic dwie sekcje (połowa)
    auto track1 = new ThalesTrackGraphic(dist / 2.0 - 0.5);
    track1->setPos(p1);
    track1->setRotation(angle);
    track1->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(track1);

    auto track2 = new ThalesTrackGraphic(dist / 2.0 - 0.5);
    QPointF mid = p1 + QPointF(dx/2.0 + 0.5*cos(angle*3.14159265358979323846/180.0), dy/2.0 + 0.5*sin(angle*3.14159265358979323846/180.0));
    track2->setPos(mid);
    track2->setRotation(angle);
    track2->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    m_scene->addItem(track2);
}

void ThalesEditorWindow::onSaveClicked() {
    QJsonArray elements;
    
    for (QGraphicsItem* item : m_scene->items()) {
        QJsonObject obj;
        obj["x"] = item->scenePos().x();
        obj["y"] = item->scenePos().y();
        obj["rot"] = item->rotation();
        
        if (auto trk = dynamic_cast<ThalesTrackGraphic*>(item)) {
            obj["type"] = "Track";
            obj["length"] = trk->length();
        } else if (auto sig = dynamic_cast<ThalesSignalGraphic*>(item)) {
            obj["type"] = "Signal";
            obj["id"] = sig->name();
            switch (sig->signalType()) {
                case ThalesSignalGraphic::TrainLeft: obj["sigType"] = "TrainLeft"; break;
                case ThalesSignalGraphic::TrainRight: obj["sigType"] = "TrainRight"; break;
                case ThalesSignalGraphic::ShuntLeft: obj["sigType"] = "ShuntLeft"; break;
                case ThalesSignalGraphic::ShuntRight: obj["sigType"] = "ShuntRight"; break;
                case ThalesSignalGraphic::TrainAndShuntLeft: obj["sigType"] = "TrainAndShuntLeft"; break;
                case ThalesSignalGraphic::TrainAndShuntRight: obj["sigType"] = "TrainAndShuntRight"; break;
            }
        } else if (auto sw = dynamic_cast<ThalesSwitchGraphic*>(item)) {
            obj["type"] = "Switch";
            obj["id"] = sw->name();
                        switch (sw->branch()) {
                case ThalesSwitchGraphic::BranchUpRight: obj["branch"] = "BranchUpRight"; break;
                case ThalesSwitchGraphic::BranchDownRight: obj["branch"] = "BranchDownRight"; break;
                case ThalesSwitchGraphic::BranchUpLeft: obj["branch"] = "BranchUpLeft"; break;
                case ThalesSwitchGraphic::BranchDownLeft: obj["branch"] = "BranchDownLeft"; break;
            }
        } else if (auto lb = dynamic_cast<ThalesLineBlockGraphic*>(item)) {
            obj["type"] = "LineBlock";
            obj["id"] = lb->label();
        } else if (auto pkpm = dynamic_cast<ThalesPkpmGraphic*>(item)) {
            obj["type"] = "Pkpm";
            // ...
        } else {
            continue;
        }
        elements.append(obj);
    }
    
    QJsonObject root;
    root["stationName"] = "Editor";
    root["elements"] = elements;
    
    QJsonDocument doc(root);
    QString filePath = "layout.json";
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        
        emit layoutExported(filePath);
    }
}

#include <QKeyEvent>
void ThalesEditorWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete) {
        if (!m_scene->selectedItems().isEmpty()) {
            for (auto item : m_scene->selectedItems()) {
                m_scene->removeItem(item);
                delete item;
            }
            clearInspector();
        }
    }
    QWidget::keyPressEvent(event);
}



void ThalesEditorWindow::setModeSelect() { m_mode = Select; }
void ThalesEditorWindow::setModeKnife() { m_mode = Knife; }
void ThalesEditorWindow::setModeMerge() { m_mode = Merge; }
void ThalesEditorWindow::toggleGrid(bool checked) {
    m_snapGrid = checked;
    m_scene->update(); // redraw grid
}


void ThalesEditorScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    
    if (m_editor && m_editor->m_mode == ThalesEditorWindow::Merge) {
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        if (auto trk = dynamic_cast<ThalesTrackGraphic*>(item)) {
            // Find adjacent track
            qreal rad = trk->rotation() * 3.14159265358979323846 / 180.0;
            // Look near the end of this track
            QPointF endPt(trk->x() + (trk->length() + 5.0) * cos(rad), trk->y() + (trk->length() + 5.0) * sin(rad));
            QGraphicsItem* adjacent = itemAt(endPt, QTransform());
            
            if (auto trk2 = dynamic_cast<ThalesTrackGraphic*>(adjacent)) {
                if (trk2 == trk) { adjacent = nullptr; trk2 = nullptr; } // prevent double free
            }
            if (auto trk2 = dynamic_cast<ThalesTrackGraphic*>(adjacent)) {
                if (trk->rotation() == trk2->rotation()) {
                    qreal newLen = trk->length() + trk2->length() + 1.0;
                    auto merged = new ThalesTrackGraphic(newLen);
                    merged->setPos(trk->x(), trk->y());
                    merged->setRotation(trk->rotation());
                    merged->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
                    
                    addItem(merged);
                    removeItem(trk);
                    removeItem(trk2);
                    delete trk;
                    delete trk2;
                    return;
                }
            }
            
            // Look near the start of this track
            QPointF startPt(trk->x() - 5.0 * cos(rad), trk->y() - 5.0 * sin(rad));
            adjacent = itemAt(startPt, QTransform());
            if (auto trk0 = dynamic_cast<ThalesTrackGraphic*>(adjacent)) {
                if (trk0 == trk) { adjacent = nullptr; trk0 = nullptr; } // prevent double free
            }
            if (auto trk0 = dynamic_cast<ThalesTrackGraphic*>(adjacent)) {
                if (trk->rotation() == trk0->rotation()) {
                    qreal newLen = trk0->length() + trk->length() + 1.0;
                    auto merged = new ThalesTrackGraphic(newLen);
                    merged->setPos(trk0->x(), trk0->y());
                    merged->setRotation(trk0->rotation());
                    merged->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
                    
                    addItem(merged);
                    removeItem(trk);
                    removeItem(trk0);
                    delete trk;
                    delete trk0;
                    return;
                }
            }
        }
    }

    if (m_editor && m_editor->m_mode == ThalesEditorWindow::Knife) {
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        if (auto trk = dynamic_cast<ThalesTrackGraphic*>(item)) {
            // Split!
            qreal clickX = event->scenePos().x();
            qreal relativeX = clickX - trk->x();
            
            // Only split if reasonably inside
            if (relativeX > 10 && relativeX < trk->length() - 10) {
                // Ensure length snaps to 10
                qreal leftLen = qRound(relativeX / 10.0) * 10.0;
                qreal rightLen = trk->length() - leftLen - 1.0; // 1px gap
                
                if (leftLen > 0 && rightLen > 0) {
                    auto leftTrk = new ThalesTrackGraphic(leftLen);
                    leftTrk->setPos(trk->x(), trk->y());
                    leftTrk->setRotation(trk->rotation());
                    leftTrk->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
                    
                    auto rightTrk = new ThalesTrackGraphic(rightLen);
                    // Right track position depends on rotation... for MVP just assume 0 for X offset
                    qreal rad = trk->rotation() * 3.14159265358979323846 / 180.0;
                    rightTrk->setPos(trk->x() + (leftLen + 1.0) * cos(rad), trk->y() + (leftLen + 1.0) * sin(rad));
                    rightTrk->setRotation(trk->rotation());
                    rightTrk->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
                    
                    addItem(leftTrk);
                    addItem(rightTrk);
                    
                    removeItem(trk);
                    delete trk;
                    return; // event handled
                }
            }
        }
    }
    QGraphicsScene::mousePressEvent(event);
}
