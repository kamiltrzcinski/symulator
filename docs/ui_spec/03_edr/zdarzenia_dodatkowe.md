# EDR - Zdarzenia Dodatkowe i Obsługa Pociągów

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję w funkcjach dodatkowych systemu EDR, wywoływanych dla pociągów z poziomu rozkładu jazdy. Każde pole, tekst, przycisk oraz okienko oparte na architekturze C++ (np. wykorzystującej zbiór widgetów Qt lub ImGui) są rozłożone na pojedyncze zdarzenia wejścia/wyjścia (mysz/klawiatura).

---

## 1. Wywołanie zdarzeń z Menu Kontekstowego
* **Co klikam:** Kliknięcie PPM (Prawym Przyciskiem Myszy) na dowolny wiersz pociągu w głównej tabeli rozkładu jazdy.
* **Co się dzieje:** Rozwija się menu kontekstowe z listą dodatkowych operacji, m.in.: `Pociąg STÓJ`, `Zgłoszenie gotowości`, `Analiza pociągu`.
* **Akcja:** Kliknięcie LPM (Lewym Przyciskiem Myszy) w konkretną opcję zamyka menu podręczne i otwiera dedykowane okno modalne na środku ekranu.

---

## 2. Okno Modalne "Rzeczywisty czas przyjazdu i odjazdu pociągu" (`TimeEntryDialog.cpp`)

Okno to otwiera się jako wyśrodkowane w pętli zdarzeń aplikacji, bez możliwości zmiany rozmiaru i ma stałe wymiary 640x650 pikseli. Tło główne okna jest jasnoszare (`#E0E0E0`).

### 2.1 Górny pasek informacyjny okna
* **Etykieta Nagłówka**: Wskazuje jakiego pociągu dotyczy wpis, np. `Pociąg 664078 / Relacja: Poznań - Warszawa`. Napis jest wyśrodkowany, napisany grubą czcionką Segoe UI 14, w ciemnozielonym kolorze (ForestGreen).

### 2.2 Panel "Przyjazd" (Górna Sekcja)
* **Tło panelu**: Szare (`#D7D7D7`). Rozmiar panelu: 560x240.
* **Etykieta "Przyjazd:"**: Gruba czcionka (Bold 16), kolor granatowy (Navy).
* **Pole daty `dtpPrzyjazd` (np. widget `QDateEdit`)**:
  * Kliknięcie LPM rozwija kalendarz. Format wyświetlania: `yyyy-MM-dd`.
* **Pola czasu `numPrzyjazdHour` oraz `numPrzyjazdMin` (np. obiekty `QSpinBox`)**:
  * Oddzielone etykietą dwukropka `:`.
  * Kliknięcie LPM w strzałkę górną/dolną w polu odpowiednio zwiększa/zmniejsza wartość godzin (0-23) lub minut (0-59). Zmiana jakiejkolwiek z tych wartości rejestruje chęć modyfikacji czasu przyjazdu pociągu.
* **Grupa kontrolek "Dodaj do czasu przyjazdu"**:
  * Zawiera siatkę przycisków dodających minuty: `[5 min.]`, `[10 min.]`, `[15 min.]` ... aż do `[200 min.]`.
  * Kliknięcie LPM w dany przycisk matematycznie podbija czas ustawiony w polach numerycznych czasu o zadeklarowaną wartość.
* **Grupa kontrolek "Dodaj do czasu przyjazdu - teksty"**:
  * Zawiera przyciski predefiniowanych opóźnień: `[Potrącenie [120 min.]]`, `[Wypadek na przejeździe [120 min.]]`, `[Uszkodzenie Sieci [70 min.]]`, `[Defekt lokomotywy [70 min.]]`.
  * Kliknięcie LPM aplikuje dany dodatek czasowy do pól powyżej (np. +120 minut) oraz (w systemie) opisuje jego przyczynę.
* **Przycisk `[Przepisz czas]`**: Kliknięcie LPM kopiuje dokładnie wartości ustawione w sekcji "Przyjazd" do niższych pól sekcji "Odjazd".

### 2.3 Panel "Odjazd" (Dolna Sekcja)
Zbudowany jest lustrzanie do Panelu Przyjazd. Tło `#D7D7D7`. Etykieta "Odjazd:" w kolorze granatowym (Navy). Posiada własny edytor daty i czasu (`dtpOdjazd`) i liczniki `QSpinBox`, jak również identyczne siatki przycisków dodających minuty oraz gotowych tekstów.

### 2.4 Dolne Przyciski Kontrolne
Posiadają zaimplementowaną unikalną, płynną animację wizualną reagującą na najechanie kursorem (enterEvent). Najechanie uruchamia obiekt Timer z pętli zdarzeń z interwałem 15ms, który klatka po klatce odrysowuje zmianę barwy tła na podstawie zmiennych postępu (np. `saveProgress`, `stojProgress`).
* **Przycisk `[Zapisz PROGNOZĘ]`**: (Logiczny cel: "Zapisz"). Kliknięcie LPM przesyła zaktualizowany czas odjazdu/przyjazdu do pociągu, instancja okna zostaje usunięta.
* **Przycisk `[Pociąg STÓJ]`**: Kliknięcie LPM wyzwala natychmiastowe zatrzymanie operacyjne. Skutek w interfejsie głównego rozkładu jazdy: Pole czasu pociągu podświetla się trwale na **kolor pomarańczowy**, informując innych dyspozytorów o opóźnieniu/postoju.
* **Przycisk `[Anuluj]`**: Zamyka całe okno i usuwa je z pamięci bez zapisu (kod odrzucenia).

---

## 3. Okno Modalne "Wprowadzanie gotowości pociągu do odjazdu" (`ReadinessDialog.cpp`)

Okno wyśrodkowuje się względem rodzica, wymiary zablokowane 500x480 pikseli. Posiada bardzo jasne, szare tło okna (`#F2F2F2`).

### 3.1 Pasek nagłówka
* Napis identyczny z Formą Czasów: `Pociąg [Numer] / Relacja: [StacjaPocz] - [StacjaKonc]`. Wyśrodkowany, kolor zielony (`#008000`), gruba czcionka 12.5.

### 3.2 Sekcja Czasu Zgłoszenia i Zgłaszającego
* **Etykieta "Gotowość pociągu do odjazdu zgłoszono:"** w kolorze granatowym (`#000080`).
* **Pole wyboru daty `dtpDate`** oraz **Pola czasu `numHour`, `numMin`**: Wyświetlają domyślnie czas bieżący systemu. Oddzielone tekstem z przecinkiem i dwukropkiem. Działają identycznie do wyżej opisanych kontrolek edycji liczb.
* **Etykieta "Zgłaszający:"** (kolor granatowy `#000080`).
* **Pole tekstowe `txtMaszynista` (np. `QLineEdit`)**: Szerokość 430px. Kliknięcie LPM powoduje pojawienie się w nim kursora mrużącego. Użytkownik wprowadza z klawiatury stopień i nazwisko osoby zgłaszającej pociąg.

### 3.3 Blok Przycisków Akcji i Parametrów
* **Przycisk `[Wprowadź analizę pociągu]`**: Szeroki na 430 pikseli, spłaszczony design (flat), jasnoszare tło (`#E1E1E1`). Kliknięcie LPM otwiera dodatkowe okno modalne analizy `AnalysisDialog`. Poprawne wypełnienie go ustawia w systemie gotowości wewnętrzną flagę `AnalysisRegistered = true`.
* **Etykieta `lblTN`**: W kolorze niebieskim. Informuje o stanie pociągu w kontekście ładunków.
* **Przycisk `[TN]` (Towary Niebezpieczne)**: Dolny, lewy róg okna. Wymiar 130x35, bez wypukłości (flat). 
  * Kliknięcie LPM emituje sygnał przełączenia statusu (ToggleDangerousGoods) – w ten sposób dyżurny zgłasza niebezpieczny ładunek.
  * Zmiana statusu od razu skutkuje zapaleniem się przycisku, a po powrocie do głównego okna aplikacji system podświetli pozycję danego pociągu na **jaskrawy, pomarańczowo-czerwony kolor**.

### 3.4 Przyciski dolnego paska
(Tutaj również zaimplementowano interwałowy obiekt Timer 15ms dla wejścia myszką w obszar przycisku, przerysowujący gradient barwy).
* **Przycisk `[Zapisz]`**: Powoduje zapis gotowości, natychmiastowe utworzenie nowej notatki w głównym Dzienniku Telefonicznym stacji według ustalonego wzorca (np. *`[Data] [Godzina] Zgłoszenie gotowości pociągu...`*), oraz usunięcie okna z zwróceniem kodu akceptacji (np. `QDialog::Accepted`).
* **Przycisk `[Anuluj]`**: Zwalnia instancję okna (odrzucenie).

---

## 4. Okno Modalne "Wprowadzania Analizy Pociągu" (`AnalysisDialog.cpp`)

Okno ma zablokowany duży wymiar: 580x800 pikseli, tło jasnoszare (`#F2F2F2`), otwiera się wyśrodkowane nad poprzednim oknem.

### 4.1 Sekcja "Dane składu" (Grupująca ramka okna)
* **Napisy etykiet:** `Brutto składu:` oraz `Długość składu:` - wyboldowane, w kolorze ciemnoczerwonym (DarkRed).
* **Pola tekstowe wprowadzania:** `txtBrutto` oraz `txtDlugosc`. Posiadają czcionkę wielkości 14 i ułożenie tekstu do prawej strony. Kliknięcie LPM pozwala na wprowadzenie liczby. 
* **Wartości planowe:** Obok pól tekstowych widnieje niebieski, pochylony tekst (np. `182`). System automatycznie przepisuje tę wartość do pola tekstowego, jeśli dotyczy to popularnych jednostek EZT i pola w momencie uruchomienia są puste.

### 4.2 Sekcja "Dane lokomotyw" (Grupująca ramka okna)
Przewiduje sloty do opisania 3 lokomotyw.
* **Etykiety wierszy:** `Lokomotywa 1:`, `Lokomotywa 2:`, `Lokomotywa 3:` - ciemnoczerwone (DarkRed).
* **Lista rozwijana `[Seria]` (np. `QComboBox`)**:
  * Naciśnięcie LPM na polu otwiera wysuwaną listę (np. EU07, ED250, EN57 itp.).
  * Naciśnięcie LPM na pożądanym elemencie zamyka listę i wrzuca go do pola, co automatycznie odświeża parametry pod spodem (Ilość osi, VMax, Trakcja), nadając im niebieski pogrubiony kolor na sztywno zaczerpnięty z modelu danych taboru.
* **Pole tekstowe `[Numer]`**: Oczekuje na ręczne wprowadzenie numeru pojazdu trakcyjnego (np. `001`).

### 4.3 Podsumowanie i Zamknięcie
* **Panel "Podsumowanie" (Grupująca ramka okna)**: Dynamicznie przelicza (nasłuchując na sygnały zmiany tekstu) i rysuje na zielono podkreśloną czcionką aktualną sumaryczną masę brutto oraz długość w miarę jak dyspozytor stuka w klawisze w górnych polach.
* **Przycisk `[Zapisz]`**: Najechanie (enterEvent) wywołuje metodę aktualizującą gradient. Kliknięcie LPM wyłuskuje dane, tworzy i układa logiczną notatkę (składnię) analizy w dzienniku ruchowym i zamyka okno.
* **Przycisk `[Anuluj]`**: Przerywa edycję i usuwa instancję okna.
