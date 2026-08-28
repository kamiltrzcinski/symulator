# Radiotelefony - Wytyczne Graficzne i Style (styles)

Ten plik zawiera specyfikację wizualną i prezentację graficzną dla trzech typów radiotelefonów.

---

## 1. Typ 1: Wielostanowiskowa Konsola (LCS Rzeszów)
* **Styl siatki stacji:** Ciemne, grafitowe tła bloków stacji (`#1A1D20`) z jasnoniebieskimi obwódkami oznaczającymi aktywne stacje.
* **Aktywny kanał (Wybrany):**
  * Zyskuje jasne, zielono-seledynowe tło z czarnym tekstem (kolor `#2ECC71`), co wyróżnia go na tle pozostałych wyszarzonych kanałów.
* **Przycisk ALARM (Radiostop):**
  * Posiada grubą, czerwoną, pulsującą obwódkę (animacja zmiany koloru obramowania od jasnoczerwonego `#FF3333` do ciemnoczerwonego `#990000`).

---

## 2. Typ 2: Konsola "Radionika" (Kartuzy)
* **Karty stacji:** Poziome bloki z zaokrąglonymi rogami (4px) i delikatną ramką o kolorze szarym.
* **Karta wybranego kanału (Pociągowy):** 
  * Posiada jaskrawożółte tło (`#FFEB3B`) z czarnymi napisami.
* **Przycisk Alarmu "A" (Lokator awarii):** 
  * Czerwony przycisk, który po wciśnięciu powoduje, że tło całej karty danej stacji zaczyna migać na czerwono w pętli.
* **Suwaki głośności (Audio/Tło):** 
  * Niestandardowy wygląd suwaków (faderów) nawiązujący do mikserów audio. Tło szyny suwaka jest czarne, a sam uchwyt suwaka (knob) ma kolor srebrny z niebieską kreską pośrodku. Najechanie na uchwyt suwaka podświetla niebieską kreskę.

---

## 3. Typ 3: Aparat biurkowy "Koliber"
* **Obudowa:** Metalowa, chropowata tekstura w kolorze głębokiej czerni z widocznymi wirtualnymi śrubami montażowymi po bokach.
* **Przycisk Alarmu "A":** 
  * Duży czerwony kwadratowy przycisk z napisem `A`. Posiada przezroczystą klapkę zabezpieczującą (nakładka graficzna o opacity 40% z odblaskiem światła). 
  * Kliknięcie najpierw unosi wirtualną klapkę (animacja obrotu o 90 stopni w górę), a drugie kliknięcie wciska przycisk.
* **Wirtualna Gruszka mikrofonowa:**
  * Gdy leży na widełkach (z boku aparatu) – rzuca delikatny cień na obudowę.
  * Po najechaniu myszką podświetla się cały jej obrys na żółto. Kliknięcie LPM "podnosi" ją (gruszka powiększa się na środku ekranu jako okno podręczne, a kabel mikrofonu rozwija się z animacją sprężyny).
  * Przytrzymanie LPM na gruszce powoduje wizualne wciśnięcie bocznego przycisku nadawania (PTT) – wskaźnik nadawania na ekranie Kolibra zapala się na czerwono.
