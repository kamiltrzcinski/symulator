import re

file_path = r"C:\Users\tymon\Desktop\SUSRK\symulator\client\src\thales\thales_browser.cpp"

with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

new_setup_browser = """void ThalesBrowserWindow::setupBrowser() {
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
    auto lblSig5 = new ThalesLabelGraphic("Zastępczy(Miga)", QColor(255, 255, 255)); lblSig5->setPos(500, sigY+15); m_scene->addItem(lblSig5);


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

    auto derail5 = new ThalesDerailGraphic("D5", ThalesDerailGraphic::Right, ThalesDerailGraphic::Placed); derail5->setOccupied(true); derail5->setPos(530, derY); m_scene->addItem(derail5);
    auto lblDer5 = new ThalesLabelGraphic("Nałożona (Zajęta)", QColor(255,0,0)); lblDer5->setPos(530, derY+15); m_scene->addItem(lblDer5);
    
    auto derail6 = new ThalesDerailGraphic("D6", ThalesDerailGraphic::Right, ThalesDerailGraphic::Clear); derail6->setOccupied(true); derail6->setPos(650, derY); m_scene->addItem(derail6);
    auto lblDer6 = new ThalesLabelGraphic("Zdjęta (Zajęta)", QColor(255,0,0)); lblDer6->setPos(650, derY+15); m_scene->addItem(lblDer6);

    // ------------------------------------------------------------------------
    // Sekcja 6: Rozjazdy (Przejścia rozjazdowe dwuczęściowe)
    // ------------------------------------------------------------------------
    addSectionHeader("6. ROZJAZDY (WSZYSTKIE STANY ORAZ POŁOŻENIA)", 20, 485);
    int swY = 535;

    // 1. Wprost (Wolna)
    auto sw1a = new ThalesSwitchGraphic("SW1a", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchNormal); sw1a->setPos(50, swY + 20); m_scene->addItem(sw1a);
    auto sw1b = new ThalesSwitchGraphic("SW1b", ThalesSwitchGraphic::BranchDownLeft, false, ThalesSwitchGraphic::SwitchNormal); sw1b->setPos(50, swY - 20); m_scene->addItem(sw1b);
    auto lblSw1 = new ThalesLabelGraphic("Wprost (Wolna)", QColor(155,155,155)); lblSw1->setPos(50, swY+45); m_scene->addItem(lblSw1);

    // 2. Bok (Wolna)
    auto sw2a = new ThalesSwitchGraphic("SW2a", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchNormal); sw2a->setDiverging(true); sw2a->setPos(200, swY + 20); m_scene->addItem(sw2a);
    auto sw2b = new ThalesSwitchGraphic("SW2b", ThalesSwitchGraphic::BranchDownLeft, false, ThalesSwitchGraphic::SwitchNormal); sw2b->setDiverging(true); sw2b->setPos(200, swY - 20); m_scene->addItem(sw2b);
    auto lblSw2 = new ThalesLabelGraphic("Bok (Wolna)", QColor(155,155,155)); lblSw2->setPos(200, swY+45); m_scene->addItem(lblSw2);

    // 3. Zastopowana
    auto sw3a = new ThalesSwitchGraphic("SW3a", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchStopped); sw3a->setPos(350, swY + 20); m_scene->addItem(sw3a);
    auto sw3b = new ThalesSwitchGraphic("SW3b", ThalesSwitchGraphic::BranchDownLeft, false, ThalesSwitchGraphic::SwitchStopped); sw3b->setPos(350, swY - 20); m_scene->addItem(sw3b);
    auto lblSw3 = new ThalesLabelGraphic("Zastopowana", QColor(255,0,255)); lblSw3->setPos(350, swY+45); m_scene->addItem(lblSw3);

    // 4. Brak Kontroli
    auto sw4a = new ThalesSwitchGraphic("SW4a", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchNoControl); sw4a->setPos(500, swY + 20); m_scene->addItem(sw4a);
    auto sw4b = new ThalesSwitchGraphic("SW4b", ThalesSwitchGraphic::BranchDownLeft, false, ThalesSwitchGraphic::SwitchNoControl); sw4b->setPos(500, swY - 20); m_scene->addItem(sw4b);
    auto lblSw4 = new ThalesLabelGraphic("Brak Kontroli", QColor(255,255,255)); lblSw4->setPos(500, swY+45); m_scene->addItem(lblSw4);

    // 5. Rozpruta (Wprost)
    auto sw5a = new ThalesSwitchGraphic("SW5a", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchDerailed); sw5a->setPos(650, swY + 20); m_scene->addItem(sw5a);
    auto sw5b = new ThalesSwitchGraphic("SW5b", ThalesSwitchGraphic::BranchDownLeft, false, ThalesSwitchGraphic::SwitchDerailed); sw5b->setPos(650, swY - 20); m_scene->addItem(sw5b);
    auto lblSw5 = new ThalesLabelGraphic("Rozpruta (Wprost)", QColor(255,0,0)); lblSw5->setPos(650, swY+45); m_scene->addItem(lblSw5);

    // 6. Rozpruta (Bok)
    auto sw6a = new ThalesSwitchGraphic("SW6a", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchDerailed); sw6a->setDiverging(true); sw6a->setPos(800, swY + 20); m_scene->addItem(sw6a);
    auto sw6b = new ThalesSwitchGraphic("SW6b", ThalesSwitchGraphic::BranchDownLeft, false, ThalesSwitchGraphic::SwitchDerailed); sw6b->setDiverging(true); sw6b->setPos(800, swY - 20); m_scene->addItem(sw6b);
    auto lblSw6 = new ThalesLabelGraphic("Rozpruta (Bok)", QColor(255,0,0)); lblSw6->setPos(800, swY+45); m_scene->addItem(lblSw6);

    // ------------------------------------------------------------------------
    // Sekcja 7: Terminal poleceń i Wskaźniki systemowe
    // ------------------------------------------------------------------------
    addSectionHeader("7. TERMINAL POLECEŃ ORAZ NAGŁÓWEK SYSTEMOWY", 20, 610);
    
    auto cmdBox = new ThalesCommandBoxGraphic(420, 75);
    cmdBox->setPos(30, 640);
    m_scene->addItem(cmdBox);

    m_headerGraphic = new ThalesHeaderGraphic();
    m_headerGraphic->setPos(500, 640);
    m_scene->addItem(m_headerGraphic);


    // ------------------------------------------------------------------------
    // Sekcja 8: Blokady Liniowe (Wszystkie 5 stanów) i Symbol Nastawni
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
"""

start_str = "void ThalesBrowserWindow::setupBrowser() {"
end_str = "QPointF ThalesSwitchGraphic::branchEndpoint() const {"

start_idx = content.find(start_str)
end_idx = content.find(end_str)

if start_idx != -1 and end_idx != -1:
    new_content = content[:start_idx] + new_setup_browser + "\n" + content[end_idx:]
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(new_content)
    print("Zaktualizowano plik.")
else:
    print("Nie znaleziono setupBrowser!")
