# Specyfikacja wprowadzania poleceń w EBI Screen 300

Dokument zawiera "łopatologiczną" (szczegółową krok po kroku) specyfikację procesu wprowadzania poleceń w systemie EBI Screen 300, z uwzględnieniem docelowej architektury implementacji w języku C++ (z wykorzystaniem mechanizmów takich jak pętla zdarzeń, `mouseMoveEvent`, `mousePressEvent`, `mouseReleaseEvent`, `keyPressEvent`).

## 1. Architektura Zdarzeń (Event Loop) w C++

Z punktu widzenia implementacji, interfejs użytkownika EBI Screen 300 opiera się na ciągłym przetwarzaniu zdarzeń w głównej pętli aplikacji (Event Loop). Każda akcja operatora (ruch myszą, kliknięcie, wciśnięcie klawisza) generuje odpowiednie zdarzenie, które jest przechwytywane przez odpowiedni handler:
- `mouseMoveEvent(QMouseEvent *event)` – odpowiedzialny za detekcję najechania (hover) oraz rysowanie żółtej ramki, a także za aktualizację kursora podczas operacji Drag & Drop.
- `mousePressEvent(QMouseEvent *event)` – obsługuje lewy przycisk myszy (LPM), wybór obiektu (rysowanie lazurowej ramki), rozpoczęcie operacji Drag & Drop oraz anulowanie (kliknięcie w tło).
- `mouseReleaseEvent(QMouseEvent *event)` – kończy operację Drag & Drop.
- `keyPressEvent(QKeyEvent *event)` – obsługuje klawisze funkcyjne (F1-F12), wpisywanie komend z klawiatury oraz klawisz Enter do akceptacji polecenia.

---

## 2. Operacje Wykonywane Myszą

### 2.1. Najeżdżanie myszą (Hover) - Żółta ramka
**Proces po stronie UI:**
Kiedy operator porusza myszką po ekranie i kursor (bez wciśniętego żadnego przycisku) znajdzie się w obszarze aktywnego obiektu (np. semafora, zwrotnicy), wokół tego obiektu natychmiast pojawia się żółta ramka. Obiekty, które nie posiadają przypisanych poleceń, nie reagują na najechanie.

**Implementacja C++:**
1. System odbiera zdarzenie w `mouseMoveEvent`.
2. Obliczana jest pozycja kursora i sprawdzane jest przecięcie z bounding boxami obiektów stacyjnych.
3. Jeśli kursor znajduje się nad obiektem sterowalnym, ustawiana jest flaga wewnętrzna `isHovered = true` dla tego obiektu (i `false` dla poprzednio podświetlonego).
4. Wywoływane jest `update()` lub `repaint()` w celu odrysowania obiektu z dodaną żółtą ramką.

### 2.2. Kliknięcie lewym przyciskiem myszy (Lazurowa ramka)
**Proces po stronie UI:**
Gdy obiekt jest otoczony żółtą ramką, wciśnięcie do oporu lewego przycisku myszy (co wiąże się z charakterystycznym "kliknięciem") zatwierdza wybór. Żółta ramka zmienia kolor na lazurowy (jasnoniebieski). Jednocześnie na ekranie otwiera się pole wyboru poleceń – lista dostępnych komend dla tego obiektu w formie przycisków (np. na biało podświetlone są polecenia aktywne/sugerowane, na łososiowo specjalne, a na szaro niedostępne).

**Implementacja C++:**
1. W momencie wciśnięcia przycisku wyzwalany jest `mousePressEvent`.
2. Algorytm sprawdza, czy event ma flagę `event->button() == Qt::LeftButton`.
3. Skrypt identyfikuje wybrany obiekt. Stan obiektu zmienia się z `isHovered` na `isSelected`.
4. Renderowana jest lazurowa ramka wokół obiektu.
5. Inicjalizowane jest wyświetlenie okna/widgetu z przyciskami dostępnych poleceń (np. pokazanie ukrytego elementu GUI z listą dostępnych komend na podstawie typu obiektu).

### 2.3. Przeskakiwanie kursora myszy i docelowy wybór
**Proces po stronie UI:**
Gdy użytkownik wybierze konkretne polecenie (klikając LPM w przycisk polecenia), wybrane polecenie pojawia się w wierszu poleceń (w lewym górnym rogu okna poleceń i komunikatów). Wtedy ramka otaczająca wybrany obiekt zmienia kolor z lazurowego na zielony. Dodatkowo po wybraniu polecenia (np. POC) kursor myszy automatycznie przeskakuje skokowo na przycisk "Wyślij".
Po zatwierdzeniu (kliknięcie "Wyślij"), kursor powraca automatycznie na swoje poprzednie miejsce (np. nad semafor).

**Implementacja C++:**
1. Wykrycie kliknięcia (`mousePressEvent`) na przycisk wyboru polecenia.
2. Zmiana ramki docelowego obiektu na kolor zielony w funkcji odrysowującej.
3. Programowe przesunięcie kursora do pozycji środka przycisku "Wyślij" z wykorzystaniem funkcji systemu okienkowego (np. `QCursor::setPos(buttonPos)` w Qt).
4. Zapisanie poprzedniej pozycji kursora do zmiennej (np. `lastMousePosition`).
5. Po wciśnięciu "Wyślij", powrotne wywołanie `QCursor::setPos(lastMousePosition)`.

### 2.4. Drag & Drop (Przeciągnij i upuść)
**Proces po stronie UI:**
Dla poleceń przebiegowych można wykorzystać przeciąganie myszą:
1. Operator wciśnięciem LPM wybiera obiekt początkowy (żółta, potem lazurowa ramka). Przycisk myszy nie zostaje zwolniony.
2. Trzymając LPM wciśnięty, operator przeciąga kursor na obiekt docelowy. 
3. W trakcie ruchu kursor myszy zmienia swój wygląd. Jeśli nie jest nad poprawnym obiektem docelowym - wyświetla specjalny znak zakazu/braku obiektu. Gdy najedzie na poprawny semafor końcowy, kursor zmienia ikonę.
4. Gdy kursor znajdzie się nad obiektem końcowym, wokół niego pojawia się żółta ramka.
5. Puszczenie LPM (Drop) wyzwala wybór obiektu końcowego – zaznaczenie zmienia się na lazurową ramkę dla końca przebiegu, a kursor przeskakuje na przycisk wyboru polecenia.

**Implementacja C++:**
1. W `mousePressEvent` przy wciśniętym LPM inicjalizowany jest tryb `dragMode = true`. Zapisywany jest obiekt początkowy.
2. W `mouseMoveEvent` (dla wciśniętego klawisza) aktualizowany jest graficzny symbol kursora zależnie od elementów znajdujących się pod spodem (mechanika Drag&Drop Framework). Jeśli pod kursorem jest odpowiedni obiekt, wywoływany jest na nim `update()` (żółta ramka).
3. W `mouseReleaseEvent` tryb `dragMode` jest kończony (`false`). Jeśli zdarzenie puszczenia wystąpiło nad poprawnym obiektem docelowym, wyzwalany jest algorytm dopisania obiektu końcowego do budowanego przebiegu i wyświetlenia dostępnych poleceń.

---

## 3. Operacje Klawiatury i Linii Poleceń

### 3.1. Przyciski Funkcyjne (F1 - F12)
**Proces po stronie UI:**
System umożliwia bindowanie poleceń do klawiszy funkcyjnych F1-F12.
Wciśnięcie wybranego klawisza funkcyjnego powoduje:
a) Pojawienie się nazwy polecenia w wierszu poleceń,
b) Ujawnienie przycisku „Wyślij”,
c) Wyświetlenie lazurowej ramki na obiektach w trybie oczekiwania, która po puszczeniu przycisku zmienia się na zieloną ramkę na obiektach, których ma dotyczyć komenda.
Zatwierdzenie odbywa się poprzez wciśnięcie klawisza `Enter` na klawiaturze lub kliknięcie przycisku "Wyślij".

**Implementacja C++:**
1. W `keyPressEvent` wykrywany jest kod klawisza (np. `Qt::Key_F1`).
2. Przypisane makro komendy zostaje wpisane do bufora wiersza poleceń. Wymuszone zostaje odrysowanie odpowiednich elementów graficznych (lazurowa ramka).
3. W `keyReleaseEvent` lazurowa ramka zamienia się na zieloną ramkę wokół obiektów docelowych.
4. Gdy wykryte zostanie zdarzenie naciśnięcia klawisza `Qt::Key_Enter` / `Qt::Key_Return`, wywoływana jest funkcja wysłania komendy do systemu zależnościowego.

### 3.2. Wpisywanie z klawiatury i Okno Wpisywania Poleceń (Linia poleceń)
**Proces po stronie UI:**
1. Wciśnięcie klawisza F12, a następnie kliknięcie lewym przyciskiem w obraz stacji otwiera na ekranie dedykowane, małe okienko (kontrolka typu ComboBox) z pustą linią do wpisywania poleceń oraz rozwijaną listą wszystkich możliwych poleceń.
2. Zaczynając pisać w oknie, system zaczyna automatycznie zawężać listę dostępnych komend (autouzupełnianie/filtrowanie predyktywne).
3. Klawiszami strzałek GÓRA/DÓŁ można poruszać się po odfiltrowanej liście poleceń.
4. Zaznaczenie komendy i wciśnięcie Enter zatwierdza polecenie i przekazuje do realizacji. 
5. Jeżeli komenda, którą wpisał operator, fizycznie nie istnieje w spisie, podświetla się ona na kolor czerwony po wciśnięciu Enter, a edycja nie zostaje przerwana.

**Implementacja C++:**
1. Wciśnięcie F12 ustawia flagę aktywującą okno ComboBox, które pojawia się w określonej pozycji ekranu i przejmuje focus (SetFocus).
2. Obsługa `textChanged` w polu QComboBox wywołuje funkcję filtrującą model danych (lista dostępnych poleceń dla tej stacji) z aktualnym ciągiem znaków.
3. Klawisze strzałek przechwytywane są w `keyPressEvent` i zmieniają `currentIndex` ComboBoxa.
4. Zdarzenie `Qt::Key_Enter` waliduje wprowadzony tekst. Jeżeli string nie znajduje się na liście modeli, pole tekstowe zmienia tło/czcionkę na czerwoną (np. `setStyleSheet("color: red;")`), jeśli jest poprawne - komenda idzie do parsera.

---

## 4. Anulowanie Wprowadzania Poleceń
**Proces po stronie UI:**
Jeśli na jakimkolwiek etapie przygotowywania polecenia (żółta ramka, lazurowa ramka, lista komend na wierzchu) użytkownik kliknie lewym przyciskiem myszy na czarnym tle (poza jakimkolwiek interaktywnym obiektem), proces wprowadzania polecenia jest natychmiast przerywany. Znikają okna komend, ramki na obiektach gasną i system wraca do stanu spoczynku.

**Implementacja C++:**
1. Wykrycie zdarzenia `mousePressEvent` w głównej planszy na czarnym tle.
2. Wywołanie funkcji `cancelCommandState()`.
3. Zresetowanie wszystkich flag: `isSelected = false`, `isHovered = false`, czyszczenie bufora poleceń.
4. Ukrycie komponentów GUI (wiersz poleceń, okno przycisków komend, zniknięcie przycisku "Wyślij").
5. Wymuszone `update()` / `repaint()`, by powrócić do widoku w stanie normalnym.
