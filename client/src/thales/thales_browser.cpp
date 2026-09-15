#include "thales/thales_browser.hpp"
#include "thales/graphics/thales_button_item.hpp"
#include "thales/graphics/thales_command_box_item.hpp"
#include "thales/graphics/thales_derail_item.hpp"
#include "thales/graphics/thales_end_catena_item.hpp"
#include "thales/graphics/thales_header_item.hpp"
#include "thales/graphics/thales_label_item.hpp"
#include "thales/graphics/thales_line_block_item.hpp"
#include "thales/graphics/thales_pkpm_item.hpp"
#include "thales/graphics/thales_platform_item.hpp"
#include "thales/graphics/thales_signal_box_item.hpp"
#include "thales/graphics/thales_signal_item.hpp"
#include "thales/graphics/thales_switch_item.hpp"
#include "thales/graphics/thales_track_item.hpp"
#include <QGraphicsTextItem>
#include <QPen>
#include <QTimer>
ThalesBrowserWindow::ThalesBrowserWindow(QWidget* parent)
    : QMainWindow(parent) {

    setWindowTitle("Thales ML8 / RSS - Przeglądarka Komponentów Pulpitu");
    resize(1080, 880);

    m_view = new QGraphicsView(this);
    m_scene = new QGraphicsScene(this);

    m_view->setScene(m_scene);
    m_view->setBackgroundBrush(QColor(33, 33, 33));
    m_view->setStyleSheet("border: none;");

    setCentralWidget(m_view);

    setupBrowser();

    // Scale całości o 40% względem domyślnego rozmiaru
    m_view->scale(1.4, 1.4);

    // Timer 500ms – odświeżanie zegara i elementów migających (FaultBlinking, Substitute, itp.)
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        m_scene->update();
    });
    timer->start(500);
}

void ThalesBrowserWindow::addSectionHeader(const QString& title, qreal x, qreal y) {
    auto header = new ThalesLabelGraphic(title, QColor(0, 200, 255), true);
    header->setPos(x, y);
    m_scene->addItem(header);

    QPen pen(QColor(60, 60, 60), 1);
    m_scene->addLine(x, y + 16, x + 980, y + 16, pen);
}

void ThalesBrowserWindow::setupBrowser() {
    // ------------------------------------------------------------------------
    // Sekcja 1: Przyciski funkcyjne
    // ------------------------------------------------------------------------
    addSectionHeader("1. PRZYCISKI FUNKCYJNE (WEKTOROWY KOD 3D, KLIKALNE)", 20, 20);

    auto btnOtski = new ThalesButtonGraphic("OTSKI1", ThalesButtonGraphic::BlueOT);
    btnOtski->setPos(30, 48); m_scene->addItem(btnOtski);

    auto btnOteskx = new ThalesButtonGraphic("OTESKX", ThalesButtonGraphic::BlueOT);
    btnOteskx->setPos(115, 48); m_scene->addItem(btnOteskx);

    auto btnOtswa = new ThalesButtonGraphic("OTSWA", ThalesButtonGraphic::BlueOT);
    btnOtswa->setPos(200, 48); m_scene->addItem(btnOtswa);

    auto btnOtpoa = new ThalesButtonGraphic("OTPOA1", ThalesButtonGraphic::BlueOT, true);
    btnOtpoa->setPos(280, 48); m_scene->addItem(btnOtpoa);

    auto btnLoff = new ThalesButtonGraphic("LOFF", ThalesButtonGraphic::GraySystem);
    btnLoff->setPos(380, 48); m_scene->addItem(btnLoff);

    auto btnHmi = new ThalesButtonGraphic("HMI", ThalesButtonGraphic::GraySystem);
    btnHmi->setPos(445, 48); m_scene->addItem(btnHmi);

    auto btnPzb = new ThalesButtonGraphic("PZB", ThalesButtonGraphic::RedPzb);
    btnPzb->setPos(510, 48); m_scene->addItem(btnPzb);

    auto lblBtnDesc = new ThalesLabelGraphic("Opis: Czysty wektorowy kod, ramka 3D, kliknij na dowolny obiekt!", QColor(130, 130, 130));
    lblBtnDesc->setPos(580, 52); m_scene->addItem(lblBtnDesc);


    // ------------------------------------------------------------------------
    // Sekcja 2: Odcinki torowe i kozły oporowe
    // ------------------------------------------------------------------------
    addSectionHeader("2. ODCINKI TOROWE (WSZYSTKIE 7 STANÓW + KOZŁY + NUMERY POCIĄGÓW)", 20, 100);

    int trkY = 135;
    auto trkFree = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free, ThalesTrackGraphic::None, "1");
    trkFree->setPos(30, trkY); m_scene->addItem(trkFree);
    auto lblTrk1 = new ThalesLabelGraphic("Wolny", QColor(155, 155, 155)); lblTrk1->setPos(30, trkY+13); m_scene->addItem(lblTrk1);

    auto trkOcc = new ThalesTrackGraphic(80, ThalesTrackGraphic::Occupied, ThalesTrackGraphic::None, "2");
    trkOcc->setPos(130, trkY); m_scene->addItem(trkOcc);
    auto lblTrk2 = new ThalesLabelGraphic("Zajęty", QColor(255, 0, 0)); lblTrk2->setPos(130, trkY+13); m_scene->addItem(lblTrk2);

    auto trkTrain = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedTrain, ThalesTrackGraphic::None, "3");
    trkTrain->setPos(230, trkY); m_scene->addItem(trkTrain);
    auto lblTrk3 = new ThalesLabelGraphic("Poc.", QColor(0, 255, 0)); lblTrk3->setPos(230, trkY+13); m_scene->addItem(lblTrk3);

    auto trkShunt = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedShunt, ThalesTrackGraphic::None, "4");
    trkShunt->setPos(330, trkY); m_scene->addItem(trkShunt);
    auto lblTrk4 = new ThalesLabelGraphic("Manewr.", QColor(255, 255, 0)); lblTrk4->setPos(330, trkY+13); m_scene->addItem(lblTrk4);

    auto trkMag = new ThalesTrackGraphic(80, ThalesTrackGraphic::MagentaRelease, ThalesTrackGraphic::None, "5");
    trkMag->setPos(430, trkY); m_scene->addItem(trkMag);
    auto lblTrk5m = new ThalesLabelGraphic("Zwaln.", QColor(255, 0, 255)); lblTrk5m->setPos(430, trkY+13); m_scene->addItem(lblTrk5m);

    auto trkFlt = new ThalesTrackGraphic(80, ThalesTrackGraphic::FaultBlinking, ThalesTrackGraphic::None, "6");
    trkFlt->setPos(530, trkY); m_scene->addItem(trkFlt);
    auto lblTrk6f = new ThalesLabelGraphic("Usterka", QColor(255, 80, 80)); lblTrk6f->setPos(530, trkY+13); m_scene->addItem(lblTrk6f);

    auto trkPre = new ThalesTrackGraphic(80, ThalesTrackGraphic::PreReset, ThalesTrackGraphic::None, "7");
    trkPre->setPos(630, trkY); m_scene->addItem(trkPre);
    auto lblTrk7p = new ThalesLabelGraphic("PreReset", QColor(139, 0, 0)); lblTrk7p->setPos(630, trkY+13); m_scene->addItem(lblTrk7p);
    
    auto trkTNumFree = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::None, "");
    trkTNumFree->setTrainNum("123456");
    trkTNumFree->setPos(730, trkY); m_scene->addItem(trkTNumFree);
    auto lblTNum1 = new ThalesLabelGraphic("Num.Wolny", QColor(155, 155, 155)); lblTNum1->setPos(730, trkY+13); m_scene->addItem(lblTNum1);

    auto trkTNumOcc = new ThalesTrackGraphic(100, ThalesTrackGraphic::Occupied, ThalesTrackGraphic::None, "");
    trkTNumOcc->setTrainNum("123456");
    trkTNumOcc->setPos(850, trkY); m_scene->addItem(trkTNumOcc);
    auto lblTNum2 = new ThalesLabelGraphic("Num.Zajęty", QColor(255, 0, 0)); lblTNum2->setPos(850, trkY+13); m_scene->addItem(lblTNum2);

    auto trkBufL = new ThalesTrackGraphic(40, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopLeft, "");
    trkBufL->setPos(980, trkY); m_scene->addItem(trkBufL);

    auto trkBufR = new ThalesTrackGraphic(40, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight, "");
    trkBufR->setPos(1040, trkY); m_scene->addItem(trkBufR);

    // ------------------------------------------------------------------------
    // Sekcja 3: Sygnalizatory w torze
    // ------------------------------------------------------------------------
    addSectionHeader("3. SYGNALIZATORY W TORZE (JEDNOLITY KOLOR SYGNAŁU)", 20, 185);

    int sigY = 225;
    
    auto trkSig1 = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free); trkSig1->setAcceptedMouseButtons(Qt::NoButton); trkSig1->setPos(30, sigY); m_scene->addItem(trkSig1);
    auto sig1 = new ThalesSignalGraphic("K1", ThalesSignalGraphic::TrainLeft, ThalesSignalGraphic::Stop, true); sig1->setPos(60, sigY); m_scene->addItem(sig1);
    auto lblSig1 = new ThalesLabelGraphic("Stój", QColor(155, 155, 155)); lblSig1->setPos(60, sigY+15); m_scene->addItem(lblSig1);

    auto trkSig2 = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedTrain); trkSig2->setAcceptedMouseButtons(Qt::NoButton); trkSig2->setPos(140, sigY); m_scene->addItem(trkSig2);
    auto sig2 = new ThalesSignalGraphic("H2", ThalesSignalGraphic::TrainRight, ThalesSignalGraphic::ProceedTrain, true); sig2->setPos(170, sigY); m_scene->addItem(sig2);
    auto lblSig2 = new ThalesLabelGraphic("Jazda P.", QColor(0, 255, 0)); lblSig2->setPos(170, sigY+15); m_scene->addItem(lblSig2);

    auto trkSig3 = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedShunt); trkSig3->setAcceptedMouseButtons(Qt::NoButton); trkSig3->setPos(250, sigY); m_scene->addItem(trkSig3);
    auto sig3 = new ThalesSignalGraphic("N1", ThalesSignalGraphic::ShuntLeft, ThalesSignalGraphic::ProceedShunt, true); sig3->setPos(280, sigY); m_scene->addItem(sig3);
    auto lblSig3 = new ThalesLabelGraphic("Manewr", QColor(255, 255, 0)); lblSig3->setPos(280, sigY+15); m_scene->addItem(lblSig3);

    auto trkSig4 = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free); trkSig4->setAcceptedMouseButtons(Qt::NoButton); trkSig4->setPos(360, sigY); m_scene->addItem(trkSig4);
    auto sig4 = new ThalesSignalGraphic("S1", ThalesSignalGraphic::TrainRight, ThalesSignalGraphic::SignalStopped, true); sig4->setPos(390, sigY); m_scene->addItem(sig4);
    auto lblSig4 = new ThalesLabelGraphic("Zatrzymany", QColor(255, 0, 255)); lblSig4->setPos(390, sigY+15); m_scene->addItem(lblSig4);

    auto trkSig5 = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free); trkSig5->setAcceptedMouseButtons(Qt::NoButton); trkSig5->setPos(470, sigY); m_scene->addItem(trkSig5);
    auto sig5 = new ThalesSignalGraphic("Z1", ThalesSignalGraphic::TrainLeft, ThalesSignalGraphic::Substitute, true); sig5->setPos(500, sigY); m_scene->addItem(sig5);
    auto lblSig5 = new ThalesLabelGraphic("Zastepczy(Miga)", QColor(255, 255, 255)); lblSig5->setPos(500, sigY+15); m_scene->addItem(lblSig5);

    // Semafor polsamoczynny z manewrowym - Stop
    auto trkSig6 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free); trkSig6->setAcceptedMouseButtons(Qt::NoButton); trkSig6->setPos(590, sigY); m_scene->addItem(trkSig6);
    auto sig6 = new ThalesSignalGraphic("H3", ThalesSignalGraphic::TrainAndShuntLeft, ThalesSignalGraphic::Stop, true); sig6->setPos(625, sigY); m_scene->addItem(sig6);
    auto lblSig6 = new ThalesLabelGraphic("Polsam+Man. Stoj", QColor(155, 155, 155)); lblSig6->setPos(625, sigY+15); m_scene->addItem(lblSig6);

    // Semafor polsamoczynny z manewrowym - Jazda pociagowa (zielony)
    auto trkSig7 = new ThalesTrackGraphic(100, ThalesTrackGraphic::RouteLockedTrain); trkSig7->setAcceptedMouseButtons(Qt::NoButton); trkSig7->setPos(730, sigY); m_scene->addItem(trkSig7);
    auto sig7 = new ThalesSignalGraphic("H4", ThalesSignalGraphic::TrainAndShuntRight, ThalesSignalGraphic::ProceedTrain, true); sig7->setPos(765, sigY); m_scene->addItem(sig7);
    auto lblSig7 = new ThalesLabelGraphic("Polsam+Man.Jazda", QColor(0, 255, 0)); lblSig7->setPos(765, sigY+15); m_scene->addItem(lblSig7);

    // Semafor polsamoczynny z manewrowym - Jazda manewrowa z pociagowym
    auto trkSig9 = new ThalesTrackGraphic(100, ThalesTrackGraphic::RouteLockedShunt); trkSig9->setAcceptedMouseButtons(Qt::NoButton); trkSig9->setPos(870, sigY); m_scene->addItem(trkSig9);
    auto sig9 = new ThalesSignalGraphic("Ms2", ThalesSignalGraphic::TrainAndShuntLeft, ThalesSignalGraphic::ProceedShunt, true); sig9->setPos(905, sigY); m_scene->addItem(sig9);
    auto lblSig9 = new ThalesLabelGraphic("Polsam Manewr+Poc.", QColor(255, 255, 0)); lblSig9->setPos(905, sigY+15); m_scene->addItem(lblSig9);


    // ------------------------------------------------------------------------
    // Sekcja 4: PKPM i Koniec Elektryfikacji
    // ------------------------------------------------------------------------
    addSectionHeader("4. PKPM (ŻÓŁTY TRÓJKĄT) ORAZ KONIEC SIECI TRAKCYJNEJ", 20, 285);
    
    int pkpmY = 325;
    auto trkPkpm1 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight); trkPkpm1->setAcceptedMouseButtons(Qt::NoButton); trkPkpm1->setPos(30, pkpmY); m_scene->addItem(trkPkpm1);
    auto pkpm108 = new ThalesPkpmGraphic(ThalesPkpmGraphic::Right, "108"); pkpm108->setPos(70, pkpmY); m_scene->addItem(pkpm108);

    auto trkPkpm2 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopLeft); trkPkpm2->setAcceptedMouseButtons(Qt::NoButton); trkPkpm2->setPos(150, pkpmY); m_scene->addItem(trkPkpm2);
    auto pkpm114 = new ThalesPkpmGraphic(ThalesPkpmGraphic::Left, "114"); pkpm114->setPos(180, pkpmY); m_scene->addItem(pkpm114);

    auto trkCat1 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight); trkCat1->setAcceptedMouseButtons(Qt::NoButton); trkCat1->setPos(300, pkpmY); m_scene->addItem(trkCat1);
    auto cat19 = new ThalesEndCatenaGraphic("19"); cat19->setPos(340, pkpmY); m_scene->addItem(cat19);
    auto pkpm19 = new ThalesPkpmGraphic(ThalesPkpmGraphic::Right); pkpm19->setPos(370, pkpmY); m_scene->addItem(pkpm19);

    auto trkCat2 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight); trkCat2->setAcceptedMouseButtons(Qt::NoButton); trkCat2->setPos(450, pkpmY); m_scene->addItem(trkCat2);
    auto cat128 = new ThalesEndCatenaGraphic("128"); cat128->setPos(490, pkpmY); m_scene->addItem(cat128);


    // ------------------------------------------------------------------------
    // Sekcja 5: Wykolejnice (Zgodne z wezel_poznanski.pdf)
    // ------------------------------------------------------------------------
    addSectionHeader("5. WYKOLEJNICE (WSZYSTKIE STANY)", 20, 385);
    int derY = 435;
    
    auto derail1 = new ThalesDerailGraphic("D1", ThalesDerailGraphic::Right, ThalesDerailGraphic::Placed); derail1->setPos(50, derY); m_scene->addItem(derail1);
    auto lblDer1 = new ThalesLabelGraphic("Nałożona (Wolna)", QColor(155,155,155)); lblDer1->setPos(50, derY+15); m_scene->addItem(lblDer1);

    auto derail2 = new ThalesDerailGraphic("D2", ThalesDerailGraphic::Right, ThalesDerailGraphic::Clear); derail2->setPos(170, derY); m_scene->addItem(derail2);
    auto lblDer2 = new ThalesLabelGraphic("Zdjęta (Wolna)", QColor(155,155,155)); lblDer2->setPos(170, derY+15); m_scene->addItem(lblDer2);

    auto derail3 = new ThalesDerailGraphic("D3", ThalesDerailGraphic::Right, ThalesDerailGraphic::NoControl); derail3->setPos(290, derY); m_scene->addItem(derail3);
    auto lblDer3 = new ThalesLabelGraphic("Brak Kontroli (Miga)", QColor(255,255,255)); lblDer3->setPos(290, derY+15); m_scene->addItem(lblDer3);

    auto derail4 = new ThalesDerailGraphic("D4", ThalesDerailGraphic::Right, ThalesDerailGraphic::Stopped); derail4->setPos(410, derY); m_scene->addItem(derail4);
    auto lblDer4 = new ThalesLabelGraphic("Zastopowana", QColor(255,0,255)); lblDer4->setPos(410, derY+15); m_scene->addItem(lblDer4);

    auto derail5 = new ThalesDerailGraphic("D5", ThalesDerailGraphic::Right, ThalesDerailGraphic::Clear); derail5->setOccupied(true); derail5->setPos(530, derY); m_scene->addItem(derail5);
    auto lblDer5 = new ThalesLabelGraphic("Zdjeta (Zajeta)", QColor(255,0,0)); lblDer5->setPos(530, derY+15); m_scene->addItem(lblDer5);


    // ------------------------------------------------------------------------
    // Sekcja 6: Rozjazdy (Wszystkie 4 stany w ulozeniu na Wprost i na Bok)
    // ------------------------------------------------------------------------
    addSectionHeader("6. ROZJAZDY (WSZYSTKIE STANY ORAZ POLOZENIA)", 20, 485);
    int swY = 535;

    auto sw1 = new ThalesSwitchGraphic("SW1", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchNormal); sw1->setPos(50, swY); m_scene->addItem(sw1);
    auto lblSw1 = new ThalesLabelGraphic("Wprost (Wolna)", QColor(155,155,155)); lblSw1->setPos(50, swY+25); m_scene->addItem(lblSw1);

    auto sw2 = new ThalesSwitchGraphic("SW2", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchNormal); sw2->setDiverging(true); sw2->setPos(200, swY); m_scene->addItem(sw2);
    auto lblSw2 = new ThalesLabelGraphic("Bok (Wolna)", QColor(155,155,155)); lblSw2->setPos(200, swY+25); m_scene->addItem(lblSw2);

    auto sw3 = new ThalesSwitchGraphic("SW3", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchStopped); sw3->setPos(350, swY); m_scene->addItem(sw3);
    auto lblSw3 = new ThalesLabelGraphic("Zastopowana", QColor(255,0,255)); lblSw3->setPos(350, swY+25); m_scene->addItem(lblSw3);

    auto sw4 = new ThalesSwitchGraphic("SW4", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchNoControl); sw4->setPos(500, swY); m_scene->addItem(sw4);
    auto lblSw4 = new ThalesLabelGraphic("Brak Kontroli", QColor(255,255,255)); lblSw4->setPos(500, swY+25); m_scene->addItem(lblSw4);

    auto sw5 = new ThalesSwitchGraphic("SW5", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchDerailed); sw5->setPos(650, swY); m_scene->addItem(sw5);
    auto lblSw5 = new ThalesLabelGraphic("Rozpruta (Wprost)", QColor(255,0,0)); lblSw5->setPos(650, swY+25); m_scene->addItem(lblSw5);

    auto sw6 = new ThalesSwitchGraphic("SW6", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchDerailed); sw6->setDiverging(true); sw6->setPos(800, swY); m_scene->addItem(sw6);
    auto lblSw6 = new ThalesLabelGraphic("Rozpruta (Bok)", QColor(255,0,0)); lblSw6->setPos(800, swY+25); m_scene->addItem(lblSw6);

    // ------------------------------------------------------------------------
    // Sekcja 7: Terminal polecen i Wskazniki systemowe
    // ------------------------------------------------------------------------
    addSectionHeader("7. TERMINAL POLECEN ORAZ NAGLOWEK SYSTEMOWY", 20, 610);
    
    auto cmdBox = new ThalesCommandBoxGraphic(420, 75);
    cmdBox->setPos(30, 640);
    m_scene->addItem(cmdBox);

    m_headerGraphic = new ThalesHeaderGraphic();
    m_headerGraphic->setPos(500, 640);
    m_scene->addItem(m_headerGraphic);


    // ------------------------------------------------------------------------
    // Sekcja 8: Blokady Liniowe (Wszystkie 5 stanow) i Symbol Nastawni
    // ------------------------------------------------------------------------
    addSectionHeader("8. BLOKADY LINIOWE ORAZ SYMBOL NASTAWNI", 20, 750);
    int blkY = 790;

    auto blk1 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Neutral, "Neutral"); blk1->setPos(30, blkY); m_scene->addItem(blk1);
    auto blk2 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Sending, "Sending"); blk2->setPos(130, blkY); m_scene->addItem(blk2);
    auto blk3 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Receiving, "Receiving"); blk3->setPos(230, blkY); m_scene->addItem(blk3);
    auto blk4 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::PermissionRequested, "PermReq(Miga)"); blk4->setPos(330, blkY); m_scene->addItem(blk4);
    auto blk5 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::EmergencyChange, "Emerg(Miga)"); blk5->setPos(430, blkY); m_scene->addItem(blk5);

    auto nast = new ThalesSignalBoxGraphic("Dz");
    nast->setPos(560, blkY-30);
    m_scene->addItem(nast);
    
    auto lblNast = new ThalesLabelGraphic("Ikona Nastawni", QColor(155, 155, 155));
    lblNast->setPos(560, blkY+10);
    m_scene->addItem(lblNast);

    m_scene->setSceneRect(0, 0, 1100, 1000);
}

QPointF ThalesSwitchGraphic::branchEndpoint() const {
    qreal endX = (m_dir == BranchUpRight || m_dir == BranchDownRight) ? 45.0 : 25.0;
    qreal endY = (m_dir == BranchUpRight || m_dir == BranchUpLeft) ? -25.0 : 25.0;
    return mapToScene(QPointF(endX, endY));
}
