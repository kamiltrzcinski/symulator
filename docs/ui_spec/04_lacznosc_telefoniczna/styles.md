# Łączność Telefoniczna - Wytyczne Graficzne i Style (styles)

Ten plik zawiera specyfikację wizualną wirtualnego telefonu stacjonarnego na biurku dyżurnego (odwzorowanie graficzne).

---

## 1. Wygląd i Tekstury (Niestandardowe elementy graficzne)
* **Obudowa aparatu:** Ciemnoszary matowy plastik (kolor `#2F3538`) z zaokrąglonymi rogami. Delikatny cień zewnętrzny (`box-shadow: 0px 4px 15px rgba(0,0,0,0.4)`) sprawia, że telefon wygląda trójwymiarowo na tle ekranu.
* **Przycisk ALARM / dioda NEW:** Półprzezroczysty czerwony plastik. Gdy świeci, zyskuje jasnoczerwoną poświatę (radialny gradient od `#FF0000` do przezroczystości, symulujący światło LED).
* **Wyświetlacz LCD:** Zielono-szare tło retro (kolor `#A3C1AD`) z czarnymi, pikselowymi czcionkami (styl czcionki *Monospace* lub *Digital*). Delikatny cień wewnętrzny sugeruje zagłębienie ekranu w obudowie.

---

## 2. Interakcje i Reakcje na Kursor (States)
* **Przyciski numeryczne i funkcyjne:**
  * **Najechanie (Hover):** Kursor zmienia się w łapkę (`pointer`). Przycisk zyskuje delikatne podświetlenie (staje się o 10% jaśniejszy).
  * **Wciśnięcie (Active/Click):** Trójwymiarowy cień pod przyciskiem zmniejsza się o 50%, a sam przycisk przesuwa się w dół o 1px, dając efekt fizycznego kliknięcia klawisza.
* **Słuchawka (Podnoszenie/Odkładanie):**
  * Najechanie na słuchawkę podświetla jej obrys na żółto.
  * Kliknięcie powoduje płynną animację rotacji i przesunięcia słuchawki (animacja obrotu o 15 stopni w lewo i przesunięcia na bok w czasie 0.25s). Gdy jest podniesiona, pod nią widać wirtualne widełki telefonu.

---

## 3. Prezentacja Stanów Linii (Diody M1 - M10)
Każdy przycisk szybkiego wybierania posiada małą diodę statusową:
* **Stan Wolny:** Dioda jest ciemnoszara, matowa.
* **Połączenie przychodzące (Dzwonienie):** Dioda miga naprzemiennie na zielono (0.5s pełnego świecenia z poświatą, 0.5s ciemna).
* **Aktywna rozmowa:** Dioda świeci ciągłym zielonym światłem z poświatą.
* **Rozmowa zawieszona (Hold):** Dioda powoli miga na zielono (1.5s świecenia, 0.5s ciemna).
