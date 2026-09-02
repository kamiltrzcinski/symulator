# Specyfikacja Wyglądu i Kolorów EBI Screen 300 (Wersja 0)
## Architektura i Implementacja C++ (Qt Framework)

Niniejszy dokument stanowi rygorystyczną specyfikację wizualną dla symulatora interfejsu EBI Screen 300, w oparciu o architekturę C++ oraz bibliotekę Qt. Został przygotowany na podstawie DTR (X-4-02851).

---

## 1. Architektura Kolorów i Podstawowe Stany (Enum)

Wszystkie kolory w systemie powinny być mapowane na stałe wartości RGB, korzystając z typów wyliczeniowych w C++. Należy unikać "hardkodowania" kolorów w metodach `paintEvent`.

```cpp
namespace EbiScreen {
    enum class ElementColor {
        Gray,           // Stan zasadniczy urządzeń
        Yellow,         // Przebiegi manewrowe (utwierdzona droga, sygnał zezwalający)
        Orange,         // Podświetlenie po inicjalizacji polecenia specjalnego
        Red,            // Zajętość, utwierdzenie sygnalizatorów (zabraniający)
        DarkRed,        // Ochrona boczna, reset licznika osi
        Green,          // Przebiegi pociągowe (utwierdzona droga, sygnał zezwalający)
        Blue,           // (Rezerwa/niezdefiniowane bazowo, ew. wewn.)
        Cyan,           // (Modry) Sterowanie lokalne, tarcze manewrowe
        Magenta,        // Zwalnianie przebiegu, zamknięcia, otwarty przejazd
        White,          // Brak danych o obiekcie
        Black           // Tło (QColor(0, 0, 0))
    };

    QColor getColor(ElementColor color) {
        switch (color) {
            case ElementColor::Gray: return QColor(128, 128, 128);
            case ElementColor::Yellow: return QColor(255, 255, 0);
            case ElementColor::Orange: return QColor(255, 165, 0);
            case ElementColor::Red: return QColor(255, 0, 0);
            case ElementColor::DarkRed: return QColor(139, 0, 0);
            case ElementColor::Green: return QColor(0, 255, 0);
            case ElementColor::Blue: return QColor(0, 0, 255);
            case ElementColor::Cyan: return QColor(0, 255, 255);
            case ElementColor::Magenta: return QColor(255, 0, 255);
            case ElementColor::White: return QColor(255, 255, 255);
            case ElementColor::Black: return QColor(0, 0, 0);
        }
        return QColor(Qt::white);
    }
}
```

### Priorytety Stanów Wyświetlania (od najwyższego do najniższego)
Każdy obiekt `QGraphicsItem` na scenie musi wdrażać maszynę stanów decydującą o finalnej barwie:
1. `Brak danych` -> Biała
2. `Poza kontrolą` -> Biało-czerwona migający na przemian (obrazy szczegółowe)
3. `Rozprucie/Awaria` -> Czerwona migająca
4. `Sygnał zastępczy/Brak kontroli położenia` -> Biała migająca
5. `Zajętość/Utwierdzenie na stój` -> Czerwona
6. `Ochrona boczna/Oczekiwanie po resecie` -> Ciemno-czerwona
7. `Przebieg pociągowy` -> Zielona
8. `Przebieg manewrowy` -> Żółta
9. `Inicjalizacja poleceń specjalnych` -> Pomarańczowa (z wyjątkami jak sygnał zastępczy, kasowanie rozprucia, wył obwodu, inicjalizacja resetu)
10. `Zwalnianie/Zamknięcia/Otwarty przejazd` -> Magenta
11. `Sterowanie lokalne` -> Modra (Cyan)
12. `Stan zasadniczy` -> Szara

---

## 2. Implementacja Migania (Blinking)

Aby w pełni odzwierciedlić specyfikację z wykorzystaniem QTimer, wprowadzamy globalny menedżer migania.
Częstotliwości (przyjęte na podstawie typowych norm SRK, np. cykl 1s):
*   `Normal Blink`: 1Hz (500ms ON, 500ms OFF) - 50/50 cycle. Stosowane do migania białego, czerwonego.
*   `Alternating Blink`: 1Hz, zmiana koloru A (Biały) i B (Czerwony) co 500ms. Stosowane do stanów "poza kontrolą".

```cpp
class BlinkManager : public QObject {
    Q_OBJECT
public:
    static BlinkManager& instance() {
        static BlinkManager instance;
        return instance;
    }

    bool isBlinkPhaseOn() const { return m_phase; }
    
signals:
    void blinkStateChanged(bool isOn);

private:
    BlinkManager() {
        m_timer = new QTimer(this);
        m_timer->setInterval(500); // Zmiana stanu co 500ms -> pełny cykl 1s
        connect(m_timer, &QTimer::timeout, this, &BlinkManager::onTimeout);
        m_timer->start();
    }
    QTimer* m_timer;
    bool m_phase = false;

    void onTimeout() {
        m_phase = !m_phase;
        emit blinkStateChanged(m_phase);
    }
};
```
Klasa renderująca, np. `BaseSymbol : public QGraphicsItem` powinna podpiąć się pod sygnał `blinkStateChanged`, wymuszając przerysowanie `update()` gdy znajduje się w odpowiednim stanie.

---

## 3. Zobrazowanie Zależności i Obiektów Stacyjnych (Szczegóły Malowania QPainter)

W klasach pochodnych od `QGraphicsItem` przeciążamy `paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)`. 

### 3.1. Tory i Grupy Torów
Reprezentacja odcinka kontroli niezajętości toru (prostokąt, linia w obrazach szczegółowych). W obrazach poglądowych tory są odpowiednio mniejsze.
*   **Tor Niezajęty (Zasadniczy):** Szary ciągły.
*   **Tor w przebiegu pociągowym:** Zielony.
*   **Tor w przebiegu manewrowym:** Żółty.
*   **Tor Zajęty:** Czerwony.
*   **Oczekujący na pierwszy przejazd (po resecie licznika):** Ciemno-czerwony.
*   **W trakcie zwalniania ręcznego przebiegu:** Magenta.
*   **Tor bez kontroli niezajętości (statyczny):** Linia cienka przerywana 20-25 (szara).
*   **Brak danych:** Biały.

### 3.2. Zwrotnice, Rozjazdy i Wykolejnice
*   **Stan Zasadniczy (Minus/Plus) niezajęty:** Szary gruby segment.
*   **Utwierdzone (pociągowe):** Zielony.
*   **Utwierdzone (manewrowe) lub Zajęte:** Żółty (manewr), Czerwony (zajętość).
*   **Przestawianie (Chwilowa utrata kontroli):** Szary migający/czerwony wg logiki.
*   **Rozprucie:** Czerwony migający węzeł centralny na rozjeździe.
*   **Utrata kontroli (rozjazd zajęty/niezajęty):** Biały znak 'X' przekreślający obwód w centralnym punkcie (lub nałożony czerwony krzyżyk).
*   **Ochrona boczna/Zablokowana:** Ciemnoczerwony (linia oporowa / zablokowany iglicą), bądź Magenta w zamknięciach indywidualnych.
*   **Rozjazdy krzyżowe (pełne i niepełne):** Rysowane krzyżujące się segmenty.
*   **Wykolejnice:** Symbol prostokąta nakładanego na tor. Wykolejnica zdjęta/założona szara (stan normalny) / czerwona (zajęta) / magenta (indywidualne zamknięcie).

### 3.3. Semafory, Tarcze Manewrowe, Tarcze Ostrzegawcze i Sygn. Powtarzające
Semafor rysowany jako zorientowany w odpowiednim kierunku trójkąt, obok którego widnieje opis żółty (semafor/tarcza m.) lub modry (tarcza manewrowa).
*   **Zasadniczy (Stój):** Szary trójkąt (Semafor, Tarcza manewrowa, Sygnał powtarzający).
*   **Sygnał Zezwalający Pociągowy:** Zielony.
*   **Sygnał Zezwalający Manewrowy:** Żółty.
*   **W utwierdzonej drodze (sygnał zabraniający):** Czerwony trójkąt.
*   **Sygnał Zastępczy (Sz):** Kolor biały migający. W rysowaniu należy użyć `BlinkManager` do naprzemiennego rysowania białego trójkąta (lub elementu X w środku) z ukrywaniem go.
*   **Zamknięcie w ochronie bocznej:** Ciemno-czerwony / Trójkąt brązowy-ciemnoczerwony.
*   **Awaria/Uszkodzony sterownik/Brak transmisji:** Czarny trójkąt z dużą, czerwoną przekątną "X" na wierzchu.
*   **Zastopowanie przez dyżurnego:** Żółte obramowanie wokół (Zastopowanie w ochronie bocznej), bądź sam semafor przekreślony odpowiednim symbolem.

### 3.4. Znaczniki Końca Przebiegu
Znacznik końca używany jako adres do utwierdzenia końca na torze szlakowym lub bocznym.
*   **Szary (Stan zasadniczy):** Droga nieutwierdzona.
*   **Zielony/Żółty (Stan aktywny):** Utwierdzona droga (zielony dla pociągowego, żółty dla manewrowego) jako koniec przebiegu.
*   **Kwadrat pomarańczowy:** Stan po zainicjalizowaniu awaryjnego zwalniania.

### 3.5. Systemy Samoczynnej Blokady Liniowej (SHL-12, Eac, Eap94)
*   **Stan Neutralny:** Szare bloki kierunkowe (strzałki do zewnątrz/wewnątrz).
*   **Żądanie ustawienia kierunku:** Żółta strzałka z przerywanymi promieniami.
*   **Ustawiony kierunek i szlak wolny:** Zielona strzałka po stronie stacji "nadawczej".
*   **Ustawiony kierunek i pociąg na szlaku:** Czerwona strzałka.
*   **Zwalnianie kierunku:** Strzałka żółta migająca.
*   **Kierunek zablokowany (STOP):** Strzałka magenta.
*   **Poza kontrolą:** Strzałka biała obramowana na czerwono (migająca biało-czerwona).

### 3.6. Przejazdy Drogowe
Reprezentowane jako szary znak odwróconego 'Y' na torze.
*   **Otwarty:** Litera magenta "Y".
*   **Zamknięty (ruch drogowy stop):** Szary "Y" z poziomą kreską.
*   **Zamknięty utwierdzony w przebiegu:** Żółty.
*   **W trakcie zmiany położenia rogatek:** Magenta, dolne segmenty (wąsy) widoczne przerywane.

### 3.7. Symbole Pracy i Sterowania Stacją (Ramki Obiektów)
-   Kliknięcie myszą: Wybranie elementu to żółta prostokątna ramka wokół obiektu (przygotowanie), cyjanowa (zielonkawa) po wybraniu przebiegu początkowego, czy seledynowa dla elementu końcowego przebiegu. W kodzie rysowane na stosie malowania na końcu jako wywołanie `drawRect` wokół podanego `boundingRect()`.

---

## 5. Przykładowa implementacja malowania (Semafor)
```cpp
void SemaforItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setRenderHint(QPainter::Antialiasing);
    
    QColor fillColor = EbiScreen::getColor(EbiScreen::ElementColor::Gray); // Default
    
    if (m_hasNoData) {
        fillColor = EbiScreen::getColor(EbiScreen::ElementColor::White);
    } else if (m_state == SemaphoreState::Awaria) {
        fillColor = EbiScreen::getColor(EbiScreen::ElementColor::Black); // Czarny podkład do X
    } else if (m_state == SemaphoreState::ZastępczySz) {
        // Miganie dla sygnału zastępczego
        fillColor = BlinkManager::instance().isBlinkPhaseOn() 
                    ? EbiScreen::getColor(EbiScreen::ElementColor::White) 
                    : EbiScreen::getColor(EbiScreen::ElementColor::Black);
    } else if (m_state == SemaphoreState::ZezwalajacyPociagowy) {
        fillColor = EbiScreen::getColor(EbiScreen::ElementColor::Green);
    } else if (m_state == SemaphoreState::UtwierdzonyZabraniajacy) {
        fillColor = EbiScreen::getColor(EbiScreen::ElementColor::Red);
    }
    
    QPolygonF triangle; // Trójkąt skierowany grotem w odpowiednią stronę
    // ... inicjalizacja punktów
    
    painter->setBrush(QBrush(fillColor));
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(triangle);
    
    // Rysowanie ewentualnego awaryjnego X
    if (m_state == SemaphoreState::Awaria) {
        QPen redPen(EbiScreen::getColor(EbiScreen::ElementColor::Red), 2);
        painter->setPen(redPen);
        painter->drawLine(triangle.at(0), triangle.at(2)); // Uproszczone rysowanie X
        painter->drawLine(triangle.at(1), QPointF(0,0)); 
    }
}
```
