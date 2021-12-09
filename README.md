# NaturalDialogSystem

Popis práce:
Ciel’om tejto práce je vytvorenie systému so stochastickým dialógovým
správaním pre herné systémy. Je navrhnutý pre prostredie Unreal Engine
4 ako herný plugin. Jeho súčast’ou je podpora pre rôzne platformy (PC,
konzoly, mobily, ...). Jadro aplikácie bude vytvorené v jazyku C++ s pod-
porou použitia v Bleuprint kóde (d’alej ako BP). Aplikácia je navrhnutá
modulárne, aby používatel’ mohol funkcionalitu jednoducho upravit’ bez
zásahu do jadra systému.
Úlohou systému bude vyhl’adávanie najvhodnejších odpovedí pre vstupy
hráˇca. V práci bude integrovaný modulárny systém, ktorý umožní vývo-
járovi vybrat’ si algoritmus pre vyhl’adávanie, prípadne generovanie, od-
povedí. Jeho súˇcast’ou bude implementovaný predvolený algoritmus, za-
ložený na vyhl’adávaní pomocou kl’úˇcových slov (keywords analysis)

Progres:
Súčasťou práce je návrh riešenia, ktoré nám trvalo najdlhšie. Výsledkom je modulárny systém,
vďaka ktorému vieme integrovať ľubovoľný počet vyhľadávacích/generovacích algoritmov pre NLP.
V práci máme implementovaný predvolený algoritmus Tf-Idf na vyhľadávanie správnych odpovedí,
ktoré sa zobrazia pre hráča v UI. Zároveň sme vytvorili algoritmy na korekciu a normalizáciu vstpných dát od hráča.
V aktuálnej verzii teda dostávame správne spracované dáta do systému, ktorý vie generovať rozumné výsledky.
Počas práce systém prešiel viacerými zmenami, vďaka ktorým sme zdokonalili správanie, zjednodušili integráciu do herného prostredia
a implementovali replikáciu, vďaka ktorej je systém pripravený na prácu v multiplayer prostrediach.
Je tiež pripravený na prácu s viacjazyčnými lokalizáciami a v súčastnej verzii podporuje spoluprácu s hernými objektami
a má predpripravenú podporu pre zvukové repliky, ktoré ešte nie sú plne inplementované.
