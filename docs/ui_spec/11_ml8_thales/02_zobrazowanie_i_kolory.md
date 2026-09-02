# Specyfikacja Zobrazowania i Kolorów - Thales ESTW L90 5

Niniejsza specyfikacja definiuje zasady rysowania interfejsu użytkownika (HMI) dla systemu Thales ESTW L90 5 w środowisku Qt (C++ / QPainter). Oparta jest o rozdziały 2.1.12 (Opis kolorów) oraz 5 (Szczegółowe informacje o elementach) Podręcznika Operatora.

---

## 1. Definicje Kolorów i Animacji (Rozdział 2.1.12)

System wykorzystuje ściśle zdefiniowaną paletę barw do reprezentacji stanów urządzeń i przebiegów. W kontekście Qt zaleca się zdefiniowanie statycznej palety kolorów (np. jako stałe `QColor` w klasie konfiguracyjnej).

### Paleta podstawowa:
1. **Szary (Gray)** - Normalny (zasadniczy) stan elementów, wolny.
   * `QColor(128, 128, 128)`
2. **Czerwony (Red)** - Zajętość odcinków, utwierdzenie sygnalizatorów w przebiegach.
   * `Qt::red` lub `QColor(255, 0, 0)`
3. **Ciemnoczerwony (Dark Red)** - Stan resetu licznika osi (oczekiwanie na przejazd pociągu).
   * `QColor(139, 0, 0)`
4. **Zielony (Green)** - Elementy drogi jazdy przebiegów pociągowych (utwierdzone).
   * `Qt::green` lub `QColor(0, 255, 0)`
5. **Żółty (Yellow)** - Elementy drogi jazdy przebiegów manewrowych, drogi ochronnej; elementy wybrane za pomocą myszy.
   * `Qt::yellow` lub `QColor(255, 255, 0)`
6. **Pomarańczowy (Orange)** - Elementy wybrane do poleceń specjalnych (wymagające potwierdzenia SPEC).
   * `QColor(255, 165, 0)`
7. **Magenta** - Zwalnianie przebiegów z opóźnieniem czasowym, zamknięcia indywidualne (zastopowanie elementu), otwarty przejazd.
   * `Qt::magenta` lub `QColor(255, 0, 255)`
8. **Biały (White)** - Brak aktualnych danych o stanie elementu (usterka komunikacji/brak statusu).
   * `Qt::white` lub `QColor(255, 255, 255)`
9. **Czarny (Black)** - Tło monitora HMI oraz wygaszone elementy.
   * `Qt::black` lub `QColor(0, 0, 0)`

### Stany animowane (Miganie):
Do realizacji stanów migania w Qt należy wykorzystać `QTimer` o interwale np. 500 ms (1 Hz), z podłączonym slotem wymuszającym `update()` widżetów. W metodzie `paintEvent()` logika opiera się na sprawdzaniu flagi określającej obecną fazę (włączony/wyłączony).
1. **Biały migający** - Sygnał zastępczy na semaforze, brak kontroli położenia zwrotnicy/wykolejnicy. (Zmienia się Biały <-> Czarny/Tło).
2. **Czerwony migający** - Rozprucie zwrotnicy, krytyczne stany awaryjne. (Zmienia się Czerwony <-> Czarny/Tło).
3. **Czerwono-biały migający** - Stany awaryjne urządzeń kontroli niezajętości (np. usterka licznika osi). Animacja może polegać na zmianie kolorów pasków ukośnych lub miganiu na przemian kolorem czerwonym i białym.
4. **Magenta-biały migający** - Stany awaryjne interfejsu HMI-IM.

---

## 2. Implementacja Elementów w Qt (QPainter)

Wszystkie obiekty wektorowe rysujemy korzystając z obiektu `QPainter`. Zaleca się ustawienie odpowiedniej grubości linii (`QPen::setWidth(5)`), używając `Qt::FlatCap` i `Qt::MiterJoin` w celu uzyskania "ostrych", technicznych krawędzi charakterystycznych dla schematów blokowych SRK.

### 2.1. Zwrotnice (Switches)

Zwrotnica w systemie Thales składa się z trzech logicznych segmentów graficznych:
1. **Segment położenia (korzeń/root)** - linia dochodząca od strony ostrza.
2. **Segment toru zwrotnego (odgałęzienia)** - dwie linie reprezentujące możliwe kierunki (lewy i prawy).
3. **Segment ostrza (iglica)** - łączy korzeń z wybranym odgałęzieniem.

**Logika rysowania w `paintEvent()`:**
* Gruby wektor z korzenia w odpowiednie odgałęzienie (stan zgodny ze zgłoszeniem w systemie).
* **Kolorystyka:** Kolor całego układu zależny jest od nałożonego przebiegu lub zajętości (szary = wolna; czerwony = zajęta; zielony/żółty = w przebiegu; magenta = zastopowana).
* **Brak kontroli położenia:** Kwadrat u podstawy zwrotnicy (`QPainter::fillRect`), rysowany na biało i poddawany animacji (niewidoczny w fazie OFF).
* **Rozprucie zwrotnicy:** Ostrze (lub cała zwrotnica) miga na czerwono. Do rysowania używamy w fazie ON pędzla `QPen(Qt::red, width)`.
* **Zamknięcie przeciw przejeżdżaniu:** Reprezentowane jest jako *element pusty w środku* (hollow). W `QPainter` efekt ten osiągniemy rysując najpierw grubą linię odpowiadającą za ramkę (np. czerwoną o grubości 6), a na niej nieco cieńszą linię w kolorze tła (czarną o grubości 2).
* **Zaznaczenie do poleceń (Orange/Yellow circle):** Na korzeniu rysowane jest wypełnione koło (`QPainter::drawEllipse`) na żółto (wybranie do zadań) lub na pomarańczowo (wymagane polecenie SPEC).

### 2.2. Wykolejnice (Derailers)

Wykolejnica symbolizowana jest poziomą linią przecinającą tor lub symbolem dwóch linii równoległych wzdłuż toru, z uwzględnieniem dodatkowego wskaźnika stanu na dole.
* **Nałożona na tor (zrzucająca):** Gruba, pojedyncza pionowa (lub pozioma zależnie od orientacji toru) kreska przerywająca ciągłość schematu toru.
* **Zdjęta z toru (przejezdna):** Dwie równoległe kreski ułożone wzdłuż toru, symulujące przejezdne szyny.
* **Kolory i miganie:** Analogiczne jak w zwrotnicach. Czerwony (zajętość, usterka), magenta (zastopowanie). 
* **Brak kontroli położenia:** Mrugający biały element w okolicy symbolu.

### 2.3. Sygnalizatory (Signals)

Sygnalizatory składają się z bazy oraz jednego lub dwóch trójkątów kierunkowych. W QPainter trójkąty rysujemy korzystając z `QPainter::drawPolygon()` podając wektor trzech `QPoint`.
* **Semafor pociągowy:** Duży trójkąt. Wypełniony na czerwono (stój), zielono (jazda) lub magenta (zastopowany). 
* **Tarcza manewrowa (bądź sygnał manewrowy na semaforze):** Mniejszy trójkąt.
  * Zezwolenie na manewr (żółty/biały).
  * Zamknięty: Ciemny (czarny na szarym tle) lub czerwony.
* **Tarcza ostrzegawcza:** Trójkąt niewypełniony. W Qt należy stworzyć poligon i obrysować go za pomocą pędzla (`QPainter::setPen(QPen(Qt::gray, ...))`), wyłączając wypełnienie (`QPainter::setBrush(Qt::NoBrush)`).
* **Sygnał zastępczy (Sz):** Duży trójkąt migający na biało. (Jeśli wyświetlany, ma priorytet nad np. zepsutą czerwoną żarówką).
* **Zaznaczenie sygnalizatora:** Podobnie jak zwrotnice – pomarańczowe lub żółte kółko rysowane jako tło pod sygnalizatorem.

### 2.4. Liczniki osi i Odcinki Torowe (Axle Counters / Track Sections)

Liczniki osi nie są rysowane jako osobne pudełka; ich stan bezpośrednio przekłada się na kolor linii toru (QPainter::drawLine).
* **Tor wolny:** Szara linia (`Qt::gray`).
* **Tor zajęty:** Czerwona linia (`Qt::red`).
* **Przebieg pociągowy:** Zielona linia (`Qt::green`).
* **Przebieg manewrowy:** Żółta linia (`Qt::yellow`).
* **Zwalnianie czasowe:** Tor zmienia kolor na magentę (`Qt::magenta`).
* **Usterka kontroli niezajętości (NRA001 / ZEROLO):** Element rysowany jako pasek migający czerwono-biały (lub prążkowany). W Qt można to zrealizować poprzez `QPen::setDashPattern()` i animację `dashOffset()` lub rysowanie linii ciągłej czerwonej i przerywanej białej na wierzchu, modyfikowanej przez `QTimer`.
* **Reset wstępny (oczekiwanie na pociąg):** Ciemnoczerwony kolor (`QColor(139, 0, 0)`). 

### 2.5. Blokady Liniowe (Line Blocks)

Blokady międzystacyjne symbolizowane są strzałkami w kierunku szlaku. W Qt rysowane jako obiekty składające się z linii (grot).
* **Stan neutralny (brak nadanego kierunku):** Szare strzałki ułożone w przeciwne strony, symbolizujące szlak.
* **Ustawiony kierunek i wyjazd dozwolony:** Strzałka na żółto.
* **Kierunek zamknięty (zastopowany):** Strzałka w kolorze magenta.
* **Żądanie pozwolenia (migająca strzałka prążkowana):** W Qt strzałkę (poligon) można wypełnić używając `QBrush` ze zdefiniowanym wzorem `Qt::BDiagPattern` w połączonych kolorach żółto-czarnym (dla żądania) lub czerwono-czarnym (awaryjna zmiana kierunku/usterka), oraz wywoływać odświeżanie z użyciem timera, aby osiągnąć efekt migania w interfejsie.
* **Awaryjne stany:** Migające na czerwono lub prążkowane groty strzałek.

---
### Rekomendacje dla modułu HMI (Architektura C++ / Qt):
1. Każdy element SRK powinien dziedziczyć po klasie bazowej np. `TrackElement : public QGraphicsItem` dla płynnego skalowania widoku lupy i podglądu obszaru w `QGraphicsView`.
2. Stany elementu powinny być enumeracjami (`enum class ElementState { Free, Occupied, RouteTrain, RouteShunt, Fault ... }`) a funkcja `paint()` powinna interpretować te stany ładując odpowiedni `QPen` i `QBrush` na podstawie wzorców wymienionych w tej specyfikacji.
3. Za miganie powinien odpowiadać jeden globalny `QTimer` w aplikacji (aby elementy nie "rozjeżdżały się" w fazach migania), emitujący sygnał `blinkTick(bool isPhaseOn)`, na który mogą reagować zdefiniowane widżety poprzez `update()`.
