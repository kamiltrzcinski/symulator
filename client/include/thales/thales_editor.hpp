#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QCheckBox>

class ThalesEditorWindow : public QWidget {
    Q_OBJECT

public:
    explicit ThalesEditorWindow(QWidget* parent = nullptr);
    ~ThalesEditorWindow() override;

signals:
    void layoutExported(const QString& jsonFilePath);

private slots:
    void onSaveClicked();
    void setModeSelect();
    void setModeKnife();
    void setModeMerge();
    void toggleGrid(bool checked);
    void onSelectionChanged();
    void onPropertyChanged();
    void onConnectSwitchesClicked();
    
    // Spawn actions
    void spawnTrack();
    void spawnSignal();
    void spawnSwitch();
    void spawnLineBlock();
    void spawnPkpm();
    void spawnEndCatena();
    void spawnPlatform();
    void spawnButton();
    void spawnCommandBox();
    void spawnHeader();
    void spawnLabel();
    void spawnSignalBox();

protected:
    void keyPressEvent(QKeyEvent* event) override;
public:
    enum Mode { Select, Knife, Merge };
    Mode m_mode = Select;
    bool m_snapGrid = true;
private:
    void setupUi();
    
    void setupPalette();
    void setupInspector();
    void clearInspector();

    QGraphicsView* m_view;
    QGraphicsScene* m_scene;
    
    // Inspector widgets
    QGroupBox* m_inspectorGroup;
    QFormLayout* m_inspectorLayout;
    
    QGraphicsItem* m_selectedItem;
    
    // Shared widgets
    QDoubleSpinBox* m_spinRotation;
};
