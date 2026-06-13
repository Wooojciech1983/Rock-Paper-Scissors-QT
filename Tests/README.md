# Testy jednostkowe — GameEngine 

Framework: **GoogleTest 1.15.0** (dostarczany przez Conan).  
Plik testowy: `GameEngineTests.cpp`  
Testowana klasa: `Backend/GameEngine` (metody statyczne, zero I/O, zero wątków)

Dla uproszczenia pominąłem testy pozostałych klas.

---

## Uruchamianie testów

### Microsoft Visual Studio 2026

Projekt używa CMake, więc otwieramy go jako **folder** (nie plik `.sln`).

**Wymagania wstępne** — przed otwarciem Visual Studio należy raz zainstalować zależności Conanem (w katalogu głównym projektu):
```powershell
conan install . --build=missing -pr:b=default
```

**Kroki:**

1. Uruchom **Microsoft Visual Studio**.
2. Wybierz **File → Open → Folder…** i wskaż katalog główny projektu.  
   Visual Studio automatycznie wykryje `CMakeLists.txt` i skonfiguruje projekt (pasek statusu pokaże „CMake generation finished").
3. Zbuduj projekt: **Build → Build All** (lub `Ctrl+Shift+B`).
4. Otwórz **Test Explorer**: **Test → Test Explorer** (`Ctrl+E, T`).
5. Kliknij **Run All Tests** (zielona strzałka „▶") — Visual Studio wykryje testy GoogleTest i wyświetli wyniki w panelu.

> **Uwaga:** Jeśli Visual Studio nie wykryje testów, sprawdź czy aktywna konfiguracja (rozwijana lista na pasku narzędzi) to `x64-Debug` lub `x64-Release`, a nie `x86`. Zmiana konfiguracji wymusza ponowną generację CMake.

---

## Grupy testów

### `MoveToChar` — konwersja ruchu na znak

| Test | Wejście | Oczekiwany wynik |
|---|---|---|
| `KnownMoves` | `Rock`, `Paper`, `Scissors` | `'r'`, `'p'`, `'s'` |
| `InvalidReturnsQuestionMark` | `Invalid` | `'?'` |

---

### `GetScore` — porównanie dwóch ruchów (m1 vs m2)

Zwracane wartości: **2** = m1 wygrywa, **1** = remis, **0** = m1 przegrywa.

| Test | Wejście | Oczekiwany wynik |
|---|---|---|
| `Ties` | r-r, p-p, s-s | 1 |
| `Wins` | r-s, p-r, s-p | 2 |
| `Losses` | r-p, p-s, s-r | 0 |
| `InvalidM1Throws` | `Invalid` vs `Rock` | rzuca `std::invalid_argument` |

---

### `EvaluateRound` — ocena pełnej rundy

Zasady punktowania:
- Jeśli gracz podał nieprawidłowy ruch → **forfeit**: gracz 0 pkt, każdy bot +2 pkt.
- Jeśli wśród wszystkich uczestników obecne są wszystkie 3 opcje (r, p, s) → **remis globalny**: wszyscy +1 pkt.
- W przeciwnym razie każda para gra round-robin przez `GetScore`. Uczestnik(cy) z największą liczbą zwycięstw dostają +2 pkt, pozostali 0 pkt.
- Jeśli nikt nie wygrał żadnego meczu (wszyscy ten sam ruch) → **draw**: wszyscy +1 pkt.

| Test | Scenariusz | Oczekiwany `Outcome` | Delty |
|---|---|---|---|
| `ForfeitInvalidInput` | Gracz `Invalid`, 2 boty | `InvalidInput` | gracz 0, boty +2 każdy |
| `PlayerWins1v1` | Rock vs [Scissors] | `PlayerWins` | gracz +2, bot 0 |
| `BotWins1v1` | Rock vs [Paper] | `BotWins` | gracz 0, bot +2 |
| `IdenticalMoveDraw` | Scissors vs [Scissors] | `Draw` | obaj +1 |
| `ThreeWayDraw` | Rock vs [Paper, Scissors] | `ThreeWayDraw` | wszyscy +1 |
| `ThreeWayDrawWithMoreBots` | Rock vs [Paper, Scissors, Rock] — 4 uczestników | `ThreeWayDraw` | wszyscy +1 |
| `TieAtTopPlayerAndBot` | Rock vs [Rock, Scissors] — gracz i bot0 remisują na szczycie | `PlayerWins` | gracz +2, bot0 +2, bot1 0 |
| `MultipleBotsWinPlayerLoses` | Paper vs [Scissors, Scissors] | `BotWins` | gracz 0, oba boty +2 |
| `EmptyBotVector` | Rock vs [] — brak par do porównania | `Draw` | `deltas=={1}` |
| `DeltasSizeMatchesParticipantCount` | Rock vs [Paper, Scissors, Rock] | — | `deltas.size() == 4` (gracz + 3 boty) |
| `MovesPreservedInResult` | Rock vs [Paper, Scissors] | — | `moves` zachowane pozycyjnie w `RoundResult` |

---

### `EvaluateRound_Tournament` — scenariusze turniejowe

`TournamentManager::RunMatch` wyłania zwycięzcę meczu wyłącznie na podstawie wartości delta, nie pola `outcome`. Logika opiera się na dwóch warunkach:
- `maxDelta == 1` → wszyscy remisują → powtórz rundę
- dokładnie jeden uczestnik z deltą == 2 → ten uczestnik awansuje
- dokładnie dwóch uczestników z deltą == 2 → konieczna dogrywka (`RunMatch` rekurencja)

Testy weryfikują ten kontrakt dla meczów 2-osobowych (duel) i 3-osobowych (koszyk).

#### Mecze 2-osobowe (duel)

| Test | Scenariusz | Kluczowe asercje |
|---|---|---|
| `Duel_Draw_MaxDeltaIsOne` | Rock vs [Rock] — ten sam ruch | `deltas=={1,1}`, `maxDelta==1` → mecz musi zostać powtórzony |
| `Duel_ExactlyOneWinner` | Rock vs [Scissors] — wyraźny zwycięzca | `deltas=={2,0}`, `countWinners==1` → uczestnik awansuje |

#### Mecze 3-osobowe (koszyk)

| Test | Scenariusz | Kluczowe asercje |
|---|---|---|
| `Triple_ClearSingleWinner` | Rock vs [Scissors, Scissors] — jeden dominuje | `deltas=={2,0,0}`, `countWinners==1` |
| `Triple_AllSameDraw_MaxDeltaIsOne` | Rock vs [Rock, Rock] — wszyscy ten sam ruch | `outcome==Draw`, `deltas=={1,1,1}`, `maxDelta==1` → powtórka |
| `Triple_ThreeWayDraw_MaxDeltaIsOne` | Rock vs [Paper, Scissors] — wszystkie 3 typy ruchów | `outcome==ThreeWayDraw`, `deltas=={1,1,1}`, `maxDelta==1` → powtórka |
| `Triple_TwoCoWinners_PlayerAndBot` | Rock vs [Rock, Scissors] — player i bot0 remisują na szczycie | `deltas=={2,2,0}`, `countWinners==2` → dogrywka między nimi |
| `Triple_TwoCoWinners_BothBots` | Scissors vs [Rock, Rock] — obaj boty wygrywają | `deltas=={0,2,2}`, `countWinners==2` → dogrywka bot-vs-bot |
