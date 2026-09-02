# Thales ESTW L90 5 - Logowanie i Zasady Ogólne (Specyfikacja UI)

Poniższa specyfikacja opisuje założenia interfejsu użytkownika (HMI) dla symulatora Thales ESTW L90 5, ze szczególnym uwzględnieniem implementacji w środowisku C++ / Qt. Styl opisu jest "łopatologiczny" – gotowy do przełożenia na architekturę obiektową.

---

## 1. Monitory (MON1 - MON5)

System obsługuje do 6 monitorów TFT (standardowo 5). Nazywane są one kolejno od lewej do prawej: **MON1, MON2, MON3, MON4, MON5**.

### Implementacja w Qt:
Zarządzanie wieloma monitorami realizowane jest za pomocą klasy `QScreen` (lub `QDesktopWidget` w starszych wersjach Qt). 
Każdy monitor to osobna instancja głównego okna (`QMainWindow`), która przechowuje referencję do swojego identyfikatora.
```cpp
// Główny menedżer ekranów
QList<QScreen*> screens = QGuiApplication::screens();
for (int i = 0; i < qMin(screens.size(), 5); ++i) {
    ThalesMonitorWindow *mon = new ThalesMonitorWindow(QString("MON%1").arg(i+1));
    mon->setGeometry(screens[i]->geometry());
    mon->showFullScreen();
}
```
Obrazy można przełączać między monitorami. Wyjątkiem jest Dziennik Ruchu (BTA), który jest przypisany do stałego monitora.

---

## 2. Struktura Obrazu Monitora

Każdy aktywny ekran (np. Lupa stacyjna lub Podgląd obszaru) dzieli się na ściśle określone strefy. 

W Qt najlepiej użyć `QGridLayout` lub `QVBoxLayout` z `QHBoxLayout`, aby sztywno wydzielić obszary, podczas gdy główny widok torów to `QGraphicsView` lub odpowiednio sformatowany `QTableView`.

*   **Strefa pól poleceń dla elementów** - (górna część, dynamiczna). Wyświetla przyciski (np. szare tło, białe/czerwone napisy) z poleceniami dostępnymi dla klikniętego elementu.
*   **Linia danych wejściowych (WE) / Linia potwierdzeń (KOM) / Linia diagnostyki (XX)** - (środkowa-górna część). Pole tekstowe, gdzie wpadają komendy (budowane z kliknięć myszy lub wpisywane z klawiatury). 
*   **Wskaźnik daty/czasu/działania** - wyświetla prawidłowość działania systemu (migające prostokąty R, G, B) i czas (połączenie z `QTimer`).
*   **Pola poleceń ogólnych** - (prawa górna część, statyczna). Pola takie jak `P` (Przetwarzaj/Enter), `LKA` (Kasowanie linii poleceń), `SPEC` (polecenia szczególnej uwagi), `PZB`, `LOFF`.
*   **Część główna obrazu** - zajmuje większość ekranu. Widok `QGraphicsView` renderujący symbole torów, zwrotnic, semaforów.

---

## 3. Obsługa Myszy i Pętla Zdarzeń (Event Loop)

System Thales jest zoptymalizowany pod obsługę myszą. 

*   **Lewy Przycisk Myszy (LMB)**: Wybór elementu (tworzy obszar aktywny - *hitbox*). Kursor w kształcie strzałki.
*   **Prawy Przycisk Myszy (RMB)**: Anulowanie / cofnięcie wprowadzania danych (odpowiednik ESC).

### Zdarzenia w Qt (łopatologicznie):
W klasie dziedziczącej po `QGraphicsView` (lub `QTableView` przy układzie siatkowym) nadpisujemy metodę zdarzeń myszy. Kliknięcie w element pobiera jego identyfikator i ładuje do linii wejściowej.

```cpp
void ThalesMainView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // 1. Znajdź element pod kursorem (Hitbox)
        QGraphicsItem *item = itemAt(event->pos());
        if (ThalesTrackElement *trackElement = dynamic_cast<ThalesTrackElement*>(item)) {
            // 2. Podświetl element (np. na żółto dla wyboru pierwszego elementu)
            trackElement->setHighlighted(true);
            
            // 3. Zaktualizuj linię danych wejściowych
            CommandLineEdit->setText(trackElement->getStationCode() + "," + trackElement->getElementName());
            
            // 4. Pokaż dostępne polecenia w Strefie Pól Poleceń
            updateCommandZone(trackElement->getAvailableCommands());
        }
    } 
    else if (event->button() == Qt::RightButton) {
        // Anulowanie (odpowiednik LKA / ESC)
        cancelCurrentCommand();
    }
}
```

### Pętla wykonywania polecenia:
1. Kliknięcie lewym klawiszem na element (np. Zwrotnicę).
2. Element pojawia się w linii `WE:`.
3. Kliknięcie lewym klawiszem na polecenie w górnej strefie (np. `PZ` - Przestawienie zwrotnicy).
4. Kliknięcie na przycisk `P` (Przetwarzaj) w strefie poleceń ogólnych (lub klawisz `Enter` na klawiaturze).
5. Jeśli komenda jest na poziomie 1 (szczególnej uwagi), element podświetli się na pomarańczowo, a operator ma 20-30 sekund na kliknięcie przycisku `SPEC` (`Ctrl+A`).

---

## 4. Logowanie i Wylogowanie

Nie ma dedykowanego, ręcznie wywoływanego polecenia do logowania (np. LOGON). 
Jeśli nikt nie jest zalogowany, poruszenie myszą lub klawiszem wywołuje prośbę o "Nazwę operatora" w linii wejściowej. Po wpisaniu nazwy i hasła system nadaje uprawnienia.

*   **Wylogowanie**: Aby wylogować się ze stanowiska, używamy polecenia ogólnego **`LOFF`**. Wpisujemy to w linii poleceń i klikamy `P` (Enter).

```cpp
void CommandProcessor::processCommand(const QString &cmd) {
    if (cmd == "LOFF") {
        sessionManager->logoutCurrentUser();
        lockUI();
        displayMessage("Wylogowano. Porusz myszą, aby się zalogować.");
    }
}
```

---

## 5. Uprawnienia: LCS a Stanowisko Lokalne

System Thales odróżnia dwa poziomy sterowania:
1.  **LCS** (Centrum Sterowania Ruchem) - nadzoruje wiele stacji/nastawnic.
2.  **Lokalne Stanowisko Operatora** - znajduje się na konkretnej stacji.

Zasada: **Tylko jeden operator może mieć w danym czasie zezwolenie na obsługę konkretnego elementu/stacji.**

W C++ można to reprezentować jako zmienną w obiekcie stacji/obszaru zarządzaną przez Mutex lub flagę kontrolną.

```cpp
class StationControlArea {
private:
    QString currentOperatorId; // ID np. "LCS_1" lub "LOCAL_A"
public:
    bool hasPermission(const QString &operatorId) {
        return currentOperatorId == operatorId;
    }
    void transferPermission(const QString &newOperatorId) {
        currentOperatorId = newOperatorId;
    }
};
```

### Przepływ przekazywania zezwoleń (Workflow UI)
W prawym górnym rogu na widoku `OTZ` (Przegląd zezwoleń) widać stacje podświetlone na zielono (masz uprawnienia) lub czerwono (ktoś inny ma uprawnienia).

1.  `ZO` (Zaoferowanie obsługi) - oddawanie sterowania.
2.  `ZPO` (Żądanie obsługi) - prośba o sterowanie, odpytywana stacja pika akustycznie.
3.  `OP` (Przejęcie obsługi) - operator, który dostał Zaoferowanie (ZO) przejmuje stację w ciągu 30 sekund.
4.  `DOP` / `DOPS` - Doraźne (wymuszone) przejęcie zezwolenia (np. przy awarii łącza).

W Qt implementuje się to za pomocą `QTimer` ustawionego na 30000 ms. Jeśli po kliknięciu `ZO` przez LCS, operator lokalny nie kliknie `OP` przed upływem czasu, uprawnienia nie zostaną zmienione. W modelu sygnałów i slotów (`SIGNALS/SLOTS`) wysyłane jest zdarzenie sieciowe pomiędzy instancjami aplikacji.
