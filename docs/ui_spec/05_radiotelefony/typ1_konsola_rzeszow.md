# Radiotelefony - Typ 1: Konsola VHF/GSM-R (LCS Rzeszów)

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję konsoli dotykowej systemu radiowego VHF/GSM-R (typ 1) stosowanej w systemie SAZ, zbudowanej w oparciu o uniwersalną pętlę zdarzeń graficznych. 

---

## 1. Główne Okno Konsoli (Widok główny)
Aplikacja pełnoekranowa lub w oknie (proporcje 16:9). Tło ciemnoszare (`#2B2B2B`), imitujące interfejsy przemysłowe.

### 1.1 Siatka Stacji (Station Grid Panel)
Główna część ekranu składa się z elastycznej siatki zawierającej pionowe panele dedykowane konkretnym stacjom/posterunkom (np. `Rzeszów`, `Trzciana`, `Rudna Wielka`).
* **Nagłówek stacji (Label):** Na samej górze panelu, na czarnym tle z białą czcionką (np. `Trzciana`). Zwykłe najechanie myszką nie wywołuje akcji.
* **Moduły kanałów VHF (Wewnątrz bloku stacji):** Poniżej nagłówka stacji znajdują się prostokątne przyciski (typu przełącznik) reprezentujące kanały.
  * **Stan Spoczynku (Nieaktywny):** Tło szare (`#404040`), tekst jasnoszary, np. `002 P Rz-2`.
  * **Najechanie (Hover):** Tło rozjaśnia się (`#505050`). Kursor zmienia się na łapkę.
  * **Kliknięcie LPM (Zaznaczenie):** Zaznacza kanał jako aktywny. Tło dostaje zieloną obwódkę (grubość 2px, `#00FF00`). Poprzednio zaznaczony kanał w tej stacji zostaje odznaczony. Odgrywa się ciche kliknięcie z głośnika komputera.
* **Przyciski funkcyjne stacji (Dolna sekcja bloku stacji):**
  * **Przycisk `[PTT]` (Push-To-Talk) dla danej stacji:** 
    * Posiada ikonę mikrofonu. Tło w stanie normalnym: czarne z zielonym paskiem LED.
    * Najechanie (Hover): Tło lekko jaśniejsze.
    * Akcja (Wciśnięcie i przytrzymanie LPM na przycisku): Pasek LED zmienia kolor na czerwony (`#FF0000`). Ikona mikrofonu robi się czerwona. Konsola przechodzi w stan nadawania na wybranym aktywnym kanale tej stacji. Rejestrowany jest dźwięk z podłączonego mikrofonu PC.
    * Puszczenie LPM (Release): Przerywa nadawanie. Dioda wraca na zielono. Odgrywany jest krótki dźwięk zwolnienia nośnej "roger beep".
  * **Przyciski `[ZEW1]`, `[ZEW2]`, `[ZEW3]`:** Kliknięcie LPM wysyła odpowiednio sygnał wywołania selektywnego (ton 1, 2, 3) na kanale przypisanym do tej stacji. Przycisk wizualnie "wdusza się" na czas emisji tonu (ok. 1 sekunda).
  * **Przycisk `[ALARM]` (Na samym dole panelu stacji):**
    * Tło jaskrawoczerwone (`#D00000`), czcionka biała pogrubiona.
    * Najechanie (Hover): Zmiana na jaśniejszy czerwony.
    * Kliknięcie LPM: Rozpoczyna nadawanie sygnału alarmowego RADIOSOTP (trzy tony). Przycisk zaczyna szybko migać białym tłem. Wszystkie pociągi w zasięgu stacji natychmiast hamują awaryjnie. System loguje to zdarzenie.
    * Ponowne kliknięcie LPM: Zatrzymuje nadawanie alarmu, powrót do normalnego koloru.

### 1.2 Dolny panel logów systemowych (LogViewer)
Czarny panel u dołu po lewej stronie ekranu (szerokość 70%). Tło czarne, zielona czcionka (`Consolas`).
* Jest to pole tekstowe tylko do odczytu, z paskiem przewijania pionowego.
* Wyświetla zdarzenia radiowe, np. `[14:02:11] Rzeszów: Odbiór na kanale 002 (SPK)`.
* Kliknięcie LPM w wewnątrz tekstu pozwala na zaznaczenie logów kursorem. Kliknięcie PPM rozwija standardowe menu systemowe (Kopiuj).

### 1.3 Główny Panel Nawigacji i Sterowania (Prawy dolny róg)
Panel ogólny (Master) wpływający na całą konsolę.
* **Przycisk `[Master PTT]` (Wielki zielony przycisk):** 
  * Wciśnięcie i przytrzymanie LPM (lub użycie zewnętrznego pedału nożnego) rozpoczyna nadawanie na *wszystkich* kanałach wybranych na *wszystkich* panelach stacji w trybie Multicast. Dioda PTT zmienia się na czerwoną. Wciśnięcie i trzymanie LPM na tym przycisku jest traktowane równoważnie jak wciśnięcie lokalnych PTT.
  * Odpuszczenie LPM przerywa nadawanie, dioda wraca do koloru szarego (zgaszona).
* **Przyciski `[Master ZEW1]`, `[Master ZEW2]`, `[Master ZEW3]`:** Kliknięcie LPM wysyła tony wywołania na wszystkich aktywnych stacjach na raz. Przez 1 sekundę przyciski "zapadają się".
* **Przycisk `[KASOWANIE]`:** 
  * Tło żółte (`#FFD700`), czarny tekst.
  * Używany podczas alarmu przychodzącego (kiedy konsola krzyczy syreną Radiostop). Kliknięcie LPM wycisza alarm lokalny na głośnikach dyżurnego, przycisk wdusza się animacją na 100ms. Syrena ustaje. Konsola powraca do cichego nasłuchu.
