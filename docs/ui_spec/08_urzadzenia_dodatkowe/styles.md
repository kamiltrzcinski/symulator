# Urządzenia Dodatkowe - Wytyczne Graficzne i Style (styles)

Ten plik zawiera specyfikację wizualną dla wirtualnych pulpitów kontrolnych RASP-UZK oraz ASDEK / DSAT.

---

## 1. Urządzenie Zdalnej Kontroli RASP-UZK

### A. Okno Ogólne RASP-UZK
* **Kolorystyka pól statusowych:**
  * Szary (brak stanu / normalny): `#B2B2B2` (neutralny szary bez odblasku).
  * Zielony (poprawny/aktywny): `#00FF00` (jaskrawy zielony z poświatą LED).
  * Pomarańczowy (sterowanie lokalne): `#FF9900` (ciepły pomarańczowy).
  * Fioletowy (ostrzeganie/stan wyjątkowy): `#800080` (głęboki fiolet).
  * Czerwony migający (usterki pilne / brak transmisji): `#FF0000` (miga na zmianę z ciemnoczerwonym `#550000` co 0.5s).
  * Czerwony stały (usterki ostrzegawcze): `#FF0000` (stały czerwony bez poświaty).

### B. Okno Szczegółowe RASP-UZK
* **Sygnalizatory drogowe (S1-S4):**
  * Lampka czerwona: gdy świeci, wokół niej pojawia się radialny gradient czerwieni imitujący prawdziwy reflektor.
  * Lampka biała: delikatne mlecznobiałe światło z poświatą.
* **Napędy rogatek (N1-N4):**
  * Prostokątne klocki napędów mają kolor fioletowy w stanie zamknięcia/otwarcia, a w czasie ruchu rogatki – fioletowy klocek miga (naprzemiennie opacity 100% i 30%).
* **Czujniki torowe (C I - C XIV):**
  * Wąskie paski na torach. Gdy pociąg najeżdża na czujnik (zajętość sekcji), sekcja toru na schemacie zmienia kolor z białego na czerwony (płynna zmiana barwy w czasie 0.1s).

---

## 2. Terminal DSAT (Diagnostyka Taboru)

### A. Ekrany raportów i tła
* **Tło raportu bez usterek:** Głęboka, butelkowa zieleń (`#006600`) z jasną, neonowo-zieloną czcionką komputerową. Daje to efekt tradycyjnego monitora kineskopowego (CRT).
* **Tło raportu alarmowego (Usterka):** Jaskrawa czerwień (`#CC0000`) z białą czcionką o stałej szerokości znaków. Dodatkowo wokół ramki raportu pojawia się czerwony, rozchodzący się cień imitujący pulsowanie ostrzegawcze.
* **Czerwone pole X (Zatwierdzenie w ERSAT):**
  * Mały kwadracik w rogu raportu. Gdy raport jest niezatwierdzony, pole miga na czerwono. Po poprawnym zapisaniu w ERSAT, pojawia się w nim czarny znak `X` na stałym szarym tle, a miganie ustaje.

### B. Przycisk Alarmu "ALARM - F1"
* Przycisk na dole ekranu. W stanie alarmu przycisk pulsuje na różowo, a po najechaniu myszką podświetla się na biało. Kliknięcie (lub wciśnięcie F1 na klawiaturze) "wciska" przycisk wizualnie (zmniejszenie cienia klawisza i przesunięcie tekstu o 1px w dół) i wyłącza miganie czerwonego tła raportu (tło staje się stałe czerwone, a alarm dźwiękowy cichnie).
