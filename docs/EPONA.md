# Epona and Hyrule's horses

**Status: decisions E1-E4 taken 2026-11-05 (below); implemented in wave 28, and the theft (point 4) with
docs/TERMINA.md; see hyrule/README.md.** It follows [DESIGN.md](DESIGN.md) (Epona), which
already settles three rules:

- Child Link can ride Epona once he knows Epona's Song, and the song works in both eras.
- Epona is stolen in MM's intro, so she is unavailable everywhere until she is recovered at Romani Ranch.
- Ingo's race is reworked around freeing the ranch and Talon's return.

## Where we start

- MM's Player rides MM's horse actor (`En_Horse`), which is OoT's `En_Horse` grown by MM's needs (Romani's escort,
  the Gorman race, the bandits). MM calls Epona with Epona's Song in its horse scenes (`z_horse.c`: Termina Field,
  Romani Ranch, the Gorman track, the swamp, Great Bay, the milk road...).
- OoT's horses are not ported: Epona as adult Link's horse (`En_Horse`), the foal at Lon Lon Ranch
  (`En_Horse_Link_Child`, which follows child Link while he plays her song), Ingo's race and its checker
  (`En_Horse_Game_Check`, `En_Ma3`), and horseback archery at Gerudo's Fortress (`En_Yabusame_Mark`, the Gerudo's
  horses). The ranch's and stable's other horses are already ported (`En_Horse_Normal`), as is Impa's horse in the
  intro dream (`En_Horse_Zelda`).
- OoT calls Epona only for adult Link, in five scenes: Hyrule Field, Lake Hylia, Gerudo Valley, Gerudo's Fortress
  and Lon Lon Ranch.

## Proposal

1. **One Epona, MM's horse actor.** Epona in Hyrule is MM's `En_Horse`, so MM's Player rides her in both worlds
   with one set of riding rules (jumps, boost carrots, mounted items). OoT's horse code is ported only where MM's
   lacks it: Ingo's race and horseback archery, as modes of that actor.
2. **Where she comes:** Epona's Song calls her in OoT's five horse scenes, for both ages, and in MM's as now.
3. **Child era:** Lon Lon Ranch keeps the rideable Epona from the start, where OoT has the foal (E2); Malon's lesson
   uses her. Once Link knows the song, Epona answers it, as DESIGN.md says.
4. **Theft:** until Link recovers Epona at Romani Ranch, the song calls nothing anywhere (DESIGN.md).
5. **Ingo's race (adult):** Link races Ingo on Epona; winning frees the ranch: Ingo gives it back, Talon returns and
   Malon is free. There is no horse prize (E3).
6. **Horseback archery** at Gerudo's Fortress keeps OoT's rules (Link on Epona, the 1000 and 1500 point prizes), for
   both ages (E4).

## Decisions

| | Question | Decision |
|-|----------|----------|
| E1 | Epona's look and size for adult Link | adult Link rides OoT's grown Epona model; child Link rides MM's |
| E2 | The foal at Lon Lon Ranch once child Link can ride Epona | no foal: the ranch has the rideable Epona from the start, and Malon's lesson uses her |
| E3 | Ingo's race | adult Link races Ingo on Epona; winning frees the ranch: Ingo gives it back, Talon returns, Malon is free; no horse prize |
| E4 | Horseback archery for child Link | both ages may take the challenge |
