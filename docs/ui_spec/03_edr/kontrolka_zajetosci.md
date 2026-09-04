# EDR - Kontrolka Zajętości Torów Stacyjnych

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję związaną z dolną kontrolką zajętości torów stacyjnych w systemie EDR opartym na architekturze C++ (np. Qt lub ImGui).

---

## 1. Wygląd Kontrolki (Dolny Pasek Zajętości)
Kontrolka znajduje się na samym dole głównego okna, pod tabelami dzienników ruchu.
* Składa się z poziomych, prostokątnych kafelków przypisanych do konkretnych numerów torów.
* Tekst na kafelkach automatycznie zmienia kolor na biały lub czarny, by zapewnić kontrast wobec koloru tła.
* **Kolory i Stany Torów:**
  * **Wolny (Zielony):** Tło kafelka to `#148741` (RGB: 20, 135, 65). Oznacza brak przypisanych pociągów lub zajętości.
  * **Zarezerwowany (Pomarańczowy):** Tło kafelka to `#EEA619` (RGB: 238, 166, 25). Oznacza zarezerwowanie pod wjazd pociągu z dziennika ruchu (wypełniona Kolumna 3, ale pusta Kolumna 6).
  * **Zajęty (Czerwony):** Tło kafelka to `#EB1616` (RGB: 235, 22, 22). Oznacza rzeczywistą zajętość toru przez pociąg lub ręczne przypisanie składu manewrującego/odstawionego.
* **Podgląd informacji (Hover):**
  * Najechanie (Hover) kursorem na kafelek (niezależnie od jego stanu) wyświetla natychmiast systemowy dymek (Tooltip/Popup).
  * W dymku wylistowane są przypisane statusy i opisy, np. `Zajęty - Wagony towarowe`, `Zarezerwowany - Pociąg 664078` lub po prostu `Wolny`.

---

## 2. Ręczne Zarządzanie Zajętościami (Interakcje)

### 2.1 Ręczne zajęcie toru (LPM)
1. **Co klikam:** Kliknięcie Lewym Przyciskiem Myszy (LPM) na wolny (zielony) kafelek toru stacyjnego.
2. **Co się dzieje:** Otwiera się dedykowane okno modalne "Zajetosc toru".

### 2.2 Manewry i Przełączanie Torów (Drag & Drop)
1. **Co robię:** Kliknięcie LPM na zajęty (czerwony) kafelek toru stacyjnego, przytrzymanie przycisku myszy i przeciągnięcie kursora nad inny, wolny (zielony) kafelek toru.
2. **Co się dzieje:** Nad kafelkiem docelowym system rysuje podgląd upuszczenia.
3. **Akcja (Puszczenie przycisku myszy):** Zwolnienie przycisku myszy na kafelku docelowym zdejmuje zajętość ze starego toru (który zmienia kolor na zielony `#148741`) i przypisuje na nowy tor (który zmienia kolor na czerwony `#EB1616`), przenosząc przy tym opis.

### 2.3 Ręczne zwalnianie toru (PPM)
1. **Co klikam:** Kliknięcie Prawym Przyciskiem Myszy (PPM) na zajęty (czerwony) lub zarezerwowany (pomarańczowy) kafelek toru.
2. **Co się dzieje:** Wywołane zostaje menu kontekstowe z opcjami odpinania pociągów lub usunięcia całej zajętości.

---

## 3. Okno Modalne "Zajetosc toru" (`TrackOccupancyDialog.cpp`)

Okno wyśrodkowuje się względem rodzica. Jest zablokowane przed zmianą rozmiaru i posiada wyłączone przyciski maksymalizacji oraz minimalizacji.
* **Rozmiar:** Stałe 420x155 pikseli.
* **Tło okna:** Jasnoszare `#F2F2F2` (RGB: 242, 242, 242).
* **Czcionka bazowa:** Segoe UI 9.5f.

### 3.1 Pasek Tytułowy i Informacje 
* Okno posiada standardowy systemowy pasek tytułowy z krzyżykiem `[X]` (kliknięcie go usuwa wskaźnik okna i przerywa zajmowanie toru).
* Etykieta Nagłówka (np. "Tor 1"): Znajduje się w punkcie 18x16, jest pogrubiona (Bold 13f) w kolorze czarnym.
* Etykieta "Pociag / opis:": Znajduje się w punkcie 20x62.

### 3.2 Sekcja Wprowadzania (pole tekstowe z opisem pociągu)
* Pole tekstowe (np. wskaźnik typu QLineEdit) zaczyna się w punkcie 155x58 i ma szerokość 220 pikseli.
* Otrzymuje styl pojedynczej ramki.
* Kliknięcie LPM aktywuje w nim kursor umożliwiając wpisanie opisu, dla którego tor ma zostać zajęty (np. tabor roboczy, lokomotywa luzem). Wpisanie pustego tekstu interpretowane jest logicznie jako "Tor Wolny".

### 3.3 Dolne Przyciski Okna
Dwa przyciski (wymiary 110x35) w dolnej części okna, oba korzystające z pętli zdarzeń i dedykowanego timera (15ms) do płynnej (interpolowanej) animacji kolorów podczas najeżdżania i odjeżdżania kursorem. Czcionka Segoe UI 9.5f Bold, kolor krawędzi `#A0A0A0` (RGB: 160,160,160).
Posiadają odrysowywane z użyciem callbacka malującego (np. QPainter) tło typu dual-split (górna i dolna połówka innego koloru):
* **Stan Spoczynku (Brak Hovera):** Górna połówka `#F5F5F5` (RGB: 245, 245, 245), dolna połówka `#D2D2D2` (RGB: 210, 210, 210).
* **Stan Aktywny (Najechanie kursorem Hover / wejście na obszar widgetu):** Rozpoczyna animację przejścia z bieżącego koloru na docelowy: Górna połówka `#FFF5B4` (RGB: 255, 245, 180), dolna połówka `#FFD250` (RGB: 255, 210, 80). Zmiana kursora na rękę.

* **Przycisk `[Zapisz]`**:
  * Pozycja na ekranie: 145x86.
  * Zaimplementowany jako domyślny przycisk powiązany z sygnałem akceptacji – aktywowany wciskając Enter na klawiaturze.
  * Kliknięcie LPM zmienia status wskazanego toru, przekazując nowo wpisany opis tekstowy i odświeżając dolny pasek głównego interfejsu (zwraca kod akceptacji np. QDialog::Accepted, a instancja okna modalnego zostaje usunięta).
* **Przycisk `[Anuluj]`**:
  * Pozycja na ekranie: 270x86.
  * Zaimplementowany jako domyślny przycisk porzucenia – sygnał zamknięcia aktywuje go również klawisz ESC.
  * Kliknięcie LPM zwalnia okno bez dokonywania żadnych modyfikacji w zajętości toru (zwraca kod odrzucenia np. QDialog::Rejected).
