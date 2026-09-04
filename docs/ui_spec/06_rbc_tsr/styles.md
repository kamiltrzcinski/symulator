# RBC / TSR - Wytyczne Graficzne i Style (styles)

Ten plik zawiera specyfikację wizualną oraz opis interakcji graficznych dla systemów Bombardier EbiScreen 300 (Ebilock) oraz autorskiego projektu ML8.

---

## 1. System EbiScreen 300 (Ebilock)

### A. Elementy Niestandardowe i Tekstury
* **Obramowania i Siatka:** Klasyczny wygląd aplikacji desktopowych Java/Windows z przełomu lat 2000/2010. Szare, płaskie ramki (kolor `#3A3A3A`) o grubości 1px rozdzielające poszczególne panele.
* **Kolorystyka wierszy:** Zgodna z tabelą w [ebilock_str1.md](file:///c:/Users/tymon/Desktop/SUSRK-UI/06_rbc_tsr/ebilock_str1.md#L48). Gradienty są wyłączone, kolory są w pełni kryjące (flat colors) dla maksymalnej czytelności na monitorach przemysłowych.
* **Licznik 120s:** Licznik sekund wyświetlany jaskrawożółtą czcionką o stałej szerokości znaków (Monospace). Poniżej 30 sekund tło pola tekstowego licznika zaczyna migać na czerwono (0.5s czerwony `#FF0000`, 0.5s domyślny szary).

### B. Reakcje na kursor i stany (Hover/Active)
* **Wybór obiektów na schemacie (Semafor/Rozjazd):**
  * Po kliknięciu `Obiekt początkowy` kursor myszy na schemacie stacji zmienia kształt na **celownik (crosshair)**.
  * Najechanie na dozwolony element (np. zwrotnicę) podświetla go cienką pomarańczową obwódką.
  * Po kliknięciu element zyskuje stałą, grubą pomarańczową obwódkę (grubość 3px, kolor `#FF9900`).
* **Przycisk Zatwierdź/Wyślij:**
  * W stanie nieaktywnym przycisk jest szary (`#555555`) z ciemnoszarym tekstem (brak reakcji na kliknięcie).
  * Po poprawnym zdefiniowaniu strefy (początek + koniec) przycisk płynnie zmienia kolor na pomarańczowy (`#FF9900`) i zyskuje efekt podświetlenia na hover (rozjaśnienie o 15%).

---

## 2. System ML8 (Mors-Siemens)

### A. Nowoczesne efekty wizualne (Modern UI)
* **Karty Pociągów:** 
  * Zaokrąglone rogi (8px), tło grafitowe z lekką przezroczystością (`rgba(30, 34, 38, 0.85)`) i rozmyciem tła pod kartą (backdrop filter blur 10px).
  * **Karta Alarmowa (Pulsowanie):** Krawędź karty alarmowej pulsuje intensywnym czerwonym światłem (radialny cień zewnętrzny `box-shadow` zmieniający promień od 2px do 12px z częstotliwością 1Hz).
* **Kapsuła Pociągu na mapie:** 
  * Płynna kapsuła z neonowym podświetleniem obrysu (glow effect) w kolorze statusu.
  * Zezwolenie na jazdę (MA) wyświetla się jako ruchome strzałki wewnątrz toru, animowane za pomocą przesunięcia tła SVG (animacja CSS `dasharray` przesuwająca się z prędkością proporcjonalną do dozwolonej prędkości pociągu).

### B. Interakcje dotykowe i myszą (Direct Manipulation)
* **Pędzel TSR (Drag & Select):**
  * Aktywacja narzędzia zmienia kursor myszy w **ikonę pędzla**.
  * Podczas przeciągania (kliknięcie i ruch myszą) nad torami wyświetla się półprzezroczysty pomarańczowy prostokąt, dopasowujący się automatycznie do geometrii toru.
  * Zwolnienie przycisku myszy natychmiast wywołuje okno pop-up.
* **Suwak (Slider) Prędkości:**
  * Duży, okrągły uchwyt suwaka (knob) o średnicy 24px (łatwy do przesunięcia palcem).
  * Przesuwanie suwaka powoduje zmianę koloru podświetlenia strefy na schemacie w czasie rzeczywistym (np. im niższa prędkość, tym intensywniejszy kolor pomarańczowo-czerwony).
