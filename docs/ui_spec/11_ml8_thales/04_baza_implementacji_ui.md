# Procedury UI i Baza Implementacji - Thales ESTW L90 5 (ML8)

Ten dokument stanowi "łopatologiczną" bazę do implementacji mechaniki interfejsu (UI) dla systemu Thales Command 900 w silniku C++ (Qt). Opisuje krok po kroku sekwencje kliknięć operatora i wymagane zdarzenia systemowe.

## 1. Architektura interakcji myszą (Pola poleceń)

W przeciwieństwie do EbiLocka (gdzie używaliśmy mechaniki "Drag & Drop"), system Thalesa opiera się na **sekwencyjnym wybieraniu elementów** i używaniu strefy "Pól Poleceń" oraz "Linii wprowadzania danych".

### Wybór pojedynczego elementu (np. zwrotnicy)
1. **Zdarzenie:** `mousePressEvent` (LPM) na obszarze aktywnym zwrotnicy.
2. **Akcja UI:** 
   - W górnej linii danych wejściowych ("WE") pojawia się kod stacji i nazwa elementu (np. `POA, Z3,`).
   - W lewej górnej części monitora (strefa Pól Poleceń) podświetlają się na niebiesko przyciski z poleceniami dostępnymi dla tego elementu (np. `PZ`, `DPZ`, `ZWS`).
3. **Zatwierdzenie komendy:**
   - Operator klika (LPM) żądane polecenie (np. `PZ`).
   - W linii wejściowej pojawia się: `POA, Z3, PZ`.
   - Następnie operator musi kliknąć przycisk zatwierdzenia `P` (Przetwarzaj) na ekranie lub wcisnąć klawisz `ENTER`.

## 2. Krok po kroku: Nastawianie przebiegu pociągowego (POC)

Aby zaprogramować w C++ nastawienie przebiegu w Thalesie, musisz obsłużyć następującą pętlę zdarzeń:

**KROK 1: Wybór semafora początkowego**
- **Działanie:** Operator klika (LPM) w semafor początkowy.
- **Reakcja Systemu:** 
  - Wokół semafora początkowego pojawia się **wypełnione żółte koło** (oznaczenie początku przebiegu).
  - Wokół wszystkich dostępnych semaforów docelowych z tego miejsca pojawiają się **puste żółte okręgi**.
  - Wewnętrzny bufor dopisuje początek trasy: `POA, A,`.

**KROK 2: Wybór semafora docelowego**
- **Działanie:** Operator klika (LPM) na jeden z podświetlonych (pustym żółtym okręgiem) semaforów docelowych.
- **Reakcja Systemu:** 
  - Żółty okrąg na elemencie docelowym zostaje **wypełniony kolorem żółtym**.
  - Żółte koło na semaforze początkowym pozostaje bez zmian.
  - Do bufora dopisywany jest cel: `POA, A, S2,`.

**KROK 3: Wybór rodzaju przebiegu (Przyciski na ekranie)**
- **Działanie:** Operator klika LPM w niebieski przycisk `POC` (Przebieg Pociągowy) w lewym górnym rogu ekranu (lub wpisuje go z klawiatury).
- **Reakcja Systemu:** 
  - Linia wejściowa przyjmuje postać: `POA, A, S2, POC`.
  - Przyszła droga przebiegu na torach jest jeszcze wyświetlana na szaro. Polecenie nie jest wysłane.

**KROK 4: Zatwierdzenie polecenia (Przycisk P / Enter)**
- **Działanie:** Operator klika LPM na kwadratowy przycisk `P` lub wciska `ENTER`.
- **Reakcja Systemu:**
  - Komenda jest wysyłana do walidacji przez moduł zależnościowy (IM).
  - Silnik systemu weryfikuje warunki (ochrona boczna, utwierdzenie, stan liczników osi).
  - Przy sukcesie: Linia "WE" zostaje wyczyszczona. Na obrazie monitora droga jazdy (tory i zwrotnice) **zmienia kolor na ZIELONY** (dla przebiegu `POC`).
  - Droga ochronna (za semaforem docelowym) **podświetla się na ŻÓŁTO**.

## 3. Procedura: Obsługa poleceń specjalnych (Przycisk SPEC)

Polecenia poziomu 1 (wymagające szczególnej uwagi, np. awaryjne zwolnienie, reset licznika osi `ZEROLO` lub doraźne przełożenie zwrotnicy `DPZ`) posiadają mechanikę autoryzacji z zegarem.

**KROK 1: Inicjalizacja komendy specjalnej**
- Operator wyklikuje cel i komendę (np. klika na licznik, wybiera `ZEROLO` i wciska `P`).
- **Reakcja UI:** Element na obrazie stacji zaczyna podświetlać się na **pomarańczowo**. System wypisuje na ekranie tekst z prośbą o autoryzację.

**KROK 2: Mechanizm QTimer (Czas na autoryzację)**
- Uruchamiany jest asynchroniczny `QTimer` w Qt z czasem 20 sekund.
- Zmienna stanu komendy przechodzi w tryb `AWAITING_SPEC`.
- Przycisk `SPEC` w strefie poleceń aktywuje się (świeci lub miga na czerwono).

**KROK 3: Zatwierdzenie (SPEC)**
- **Działanie:** Operator w ciągu 20 sekund musi kliknąć przycisk `SPEC` myszką, LUB nacisnąć `CTRL + A` na klawiaturze.
- **Reakcja Systemu:**
  - Timer zostaje zabity (`timer->stop()`).
  - Pomarańczowe podświetlenie znika. 
  - Komenda jest przekazywana do modułu wykonawczego. 
  - Zmiana kolorystyki elementu zgodnie z pożądanym stanem.

**KROK 4: Timeout (Brak autoryzacji)**
- Jeśli operator nie kliknie `SPEC` w ciągu 20 sekund, funkcja połączona z `QTimer::timeout` kasuje cały proces.
- Znika pomarańczowe podświetlenie, przycisk `SPEC` gaśnie, a w linii komunikatów pojawia się "Odrzucono".

## 4. Anulowanie wprowadzania (Escape / Klik w tło)**
W każdym momencie (przed wciśnięciem `P`) kliknięcie PPM (Prawym Przyciskiem Myszy) w puste szare tło obszaru roboczego LUB wciśnięcie klawisza `ESC` wyzwala funkcję anulującą. Czyści ona linię wejściową, usuwa żółte podświetlenia selekcji i gasi niebieskie przyciski komend.
