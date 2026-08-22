# POLSKI

## Qt6

Aplikacje `symulator-client`, `symulator-editor` oraz biblioteka `libtrackview`
korzystają z następujących modułów Qt6 (wersja 6.10.3): Core, Gui, Widgets,
Network, Sql, OpenGLWidgets, Test. Narzędzia deweloperskie w `tools/`
(licencjonowane osobno — patrz `tools/COPYING`) korzystają z modułów Core,
Gui, Widgets, Test.

Qt jest udostępniane przez The Qt Company na licencji GNU Lesser General
Public License w wersji 3 (LGPLv3), obok licencji GPL i komercyjnej. Powyższe
komponenty tego repozytorium używają Qt na warunkach LGPLv3.

Qt jest linkowane **dynamicznie** (biblioteki współdzielone `.so`/`.dll`
dostarczane obok plików wykonywalnych, nie wkompilowane statycznie). Zgodnie
z LGPLv3 §4 pozwala to na używanie Qt w oprogramowaniu własnościowym o
zamkniętym kodzie źródłowym bez obowiązku udostępniania kodu obiektowego tego
oprogramowania do relinkowania — wystarczy, że biblioteka Qt pozostaje
podmienialna przez użytkownika końcowego.

Treść licencji LGPLv3: https://www.gnu.org/licenses/lgpl-3.0.html
Źródła Qt (wersja 6.10.3): https://code.qt.io/cgit/qt/qtbase.git
Obowiązki wynikające z LGPLv3 przy dystrybucji Qt open source:
https://www.qt.io/licensing/open-source-lgpl-obligations

Niniejsza notatka dotyczy wyłącznie komponentów Qt6 — nie zmienia ani nie
rozszerza warunków licencyjnych własnego kodu tego repozytorium, opisanych w
`LICENSE.md`.

---

# ENGLISH

## Qt6

`symulator-client`, `symulator-editor`, and the `libtrackview` library use the
following Qt6 modules (version 6.10.3): Core, Gui, Widgets, Network, Sql,
OpenGLWidgets, Test. The developer tools under `tools/` (separately licensed —
see `tools/COPYING`) use Core, Gui, Widgets, Test.

Qt is made available by The Qt Company under the GNU Lesser General Public
License version 3 (LGPLv3), alongside GPL and commercial licensing options.
The components above use Qt under the LGPLv3 terms.

Qt is linked **dynamically** (shared `.so`/`.dll` libraries shipped alongside
the executables, not compiled into them). Under LGPLv3 §4, this allows using
Qt in closed-source, proprietary software without an obligation to provide
this software's own object code for relinking — it is sufficient that the Qt
library itself remains replaceable by the end user.

LGPLv3 license text: https://www.gnu.org/licenses/lgpl-3.0.html
Qt source code (version 6.10.3): https://code.qt.io/cgit/qt/qtbase.git
LGPLv3 open-source obligations when distributing Qt:
https://www.qt.io/licensing/open-source-lgpl-obligations

This notice covers Qt6 components only — it does not modify or extend the
licensing terms of this repository's own code, described in `LICENSE.md`.
