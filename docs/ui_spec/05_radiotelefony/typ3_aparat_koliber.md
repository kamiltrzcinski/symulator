# Radiotelefony - Typ 3: Biurkowy Aparat "Koliber"

Niniejsza specyfikacja opisuje w sposób skrajnie analityczny (łopatologiczny) każdy element wizualny i interakcję wirtualnego biurkowego aparatu radiotelefonicznego "Koliber" VHF/GSM-R, zbudowanego przy użyciu wirtualnego rysowania i zarządzania zdarzeniami (np. jak w Qt). Urządzenie reprezentowane jest graficznie w rzucie z przodu (2D).

---

## 1. Wygląd Zewnętrzny (Obudowa i Ekran)
Grafika urządzenia przedstawia czarną obudowę z tworzywa sztucznego ze sprzętowymi przyciskami (reprezentowanymi wizualnie w UI) oraz mikrofonogłośnikiem na kablu.

### 1.1 Mikrofonogłośnik (Gruszka, po prawej stronie)
* **Grafika:** Wirtualna "gruszka" powieszona na widełkach bocznych urządzenia. Z boku posiada czarny przycisk `PTT`.
* **Stan Spoczynku:** Gruszka zawieszona. Urządzenie pozostaje wyciszone na lokalne dźwięki tła pokoju.
* **Podniesienie Gruszki (Kliknięcie LPM w grafikę gruszki w stanie zawieszonym):**
  * Gruszka zostaje graficznie uniesiona o kilka pikseli, ukazując "zdjęcie" jej z widełek.
  * Słychać systemowe "pstryk". Z głośnika komputera zaczyna płynąć cichy biały szum z eteru kanału (urządzenie "budzi się" do nasłuchu głośnomówiącego z bliska).
* **Nadawanie PTT (Gdy gruszka jest podniesiona):**
  * Wciśnięcie i PRZYTRZYMANIE Lewego Przycisku Myszy (LPM) dokładnie na bocznym czarnym przycisku PTT na grafice gruszki:
    * Przycisk wciska się wizualnie do wewnątrz.
    * Z głośnika leci "Roger beep". Na wyświetlaczu LCD zapala się ikona anteny nadawczej i napis `NADAWANIE`. Włącza się mikrofon komputera, rejestruje głos.
  * Puszczenie LPM (Release): Wyłącza tryb nadawania, przycisk PTT odskakuje.
* **Odłożenie Gruszki (Kliknięcie LPM na obszar gruszki, gdy jest podniesiona):**
  * Gruszka wraca na widełki. Słychać "pstryk". Nasłuch szumu ustaje.

### 1.2 Ekran LCD (Środkowa górna sekcja)
* Monochromatyczny ekran, niebieskie podświetlenie (`#64B5F6`), czarne piksele (czcionka pikselowa, np. `VT323`).
* W centralnej części duży napis z numerem kanału, np. `KANAŁ: 010`. Poniżej nazwa, np. `Radiostop UHR`.
* W dolnej linii ekranu 5 krótkich tekstów wyrównanych bezpośrednio nad 5 fizycznymi przyciskami poniżej (np. `ZASIL`, `ZEM`, `REJES`, `SZUK`, `MENU`).
* W prawym górnym rogu ikona zasięgu sieci (słupki) - zablokowana do odczytu (statyczna).

### 1.3 Przycisk ALARM "A" (Lewa górna strona frontu)
* Zabezpieczony wirtualną klapką.
* **Najechanie (Hover):** Kursor zmienia się w łapkę.
* **Kliknięcie LPM na klapkę:** Klapka otwiera się z animacją odsłaniając czerwony przycisk z białą literą "A". Drugie kliknięcie zamyka klapkę.
* **Wciśnięcie i przytrzymanie LPM (długie kliknięcie, >1s) na przycisku "A" (Gdy klapka jest otwarta):**
  * Dźwięk fizycznego wciśnięcia ciężkiego przycisku.
  * Ekran LCD zaczyna migać negatywem (czarne tło, niebieski napis `* RADIOSTOP *`).
  * Urządzenie generuje w głośnikach komputera sygnał Radiostop z dużą głośnością. Czerwona dioda LED obok przycisku "A" zaczyna agresywnie migać.
  * Wszystkie pociągi symulowane na szlaku zaczynają hamować awaryjnie.
* **Przerwanie alarmu (Kliknięcie LPM na migającym przycisku "A"):** Zatrzymuje generację alarmu i wraca do widoku `KANAŁ: 010`. Klapkę można zamknąć.

### 1.4 Przyciski funkcyjne pod wyświetlaczem (5 sztuk, jasnoszare)
* Najechanie na nie robi lekkie przyciemnienie (Hover).
* **Kliknięcie LPM na dowolny przycisk (np. `MENU` - skrajny prawy):** 
  * Wciska wizualnie przycisk. Generuje piknięcie z głośnika błędu (gdyż akcja menu może być nieaktywna) lub otwiera pod-menu na ekranie LCD, zamieniając 5 opisów ekranowych na nowe funkcje (np. `GŁOŚ`, `JASNO`, `KONTR`, `WYJDŹ`). Naciśnięcie `WYJDŹ` wraca ekran do pierwotnego menu.

### 1.5 Klawiatura Numeryczna (Prawa strona urządzenia)
Klawiatura w układzie klasycznym: `1, 2, 3`, `4, 5, 6`, `7, 8, 9`, `* , 0 , #`, oraz na dole dwa przyciski funkcyjne: `[X] (Anuluj, czerwony)` i `[V] (Potwierdź, zielony)`.
* **Kliknięcie LPM w cyfry (np. wpisanie 0, 0, 7):**
  * Przy każdym kliknięciu cyfra "zapada się" i słychać krótki ton (BEEP).
  * Na ekranie LCD pojawia się zachęta: `WYBIERZ KANAŁ: 007_` (kursor miga).
* **Kliknięcie LPM w zielony przycisk `[V]`:**
  * Jeśli wprowadzony kanał (np. 007) istnieje, odzywa się "Success Beep". Ekran wraca do ekranu głównego, nadpisując stary kanał na `KANAŁ: 007`. Użytkownik znajduje się na nowym paśmie nasłuchu.
  * Jeśli kanał nie istnieje, odzywa się potrójny zły sygnał (Error Beep), ekran miga `BŁĘDNY KANAŁ` i powraca do stanu wpisywania.
* **Kliknięcie LPM w czerwony przycisk `[X]` (w trakcie wpisywania):**
  * Przerywa operację zmiany kanału. Ekran wraca do uprzednio wybranego kanału bez zmian.
