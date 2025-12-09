# Disco club
### Juraj Majernik; Valeriia Petrishka

---
## Tasks

### 3D objekty
- [X] (2b) Objekty ako unikátne 3D polygonálne modely 
- [X] (2b) Unikátne mapovanie textúr pomocou UV koordinátov na 3D objekt
- [x] (2b) Efektívne inštancovanie 3D objektu: zobrazenie aspoň 5000 inštancií za použitia OpenGL inštancovania
- [X] (2b) Priesvitné objekty
- [X] (1b) Objekt s využitím hrbolatej textúry (normal mapping)

### Scéna
- [X] (2b) Procedurálne generovaná scéna
- [X] (4b) Hierarchická reprezentácia scény
- [X] (2b) Využitie techniky mapovania na kocku (cube mapping)

### Animované objekty
- [X] (2b) Procedurálna animácia
- [X] (3b) Animácia na základe dát uložených v kľúčových snímkoch a interpolácie
- [X] (1b) Animácia riadená komplexnými animačnými krivkami pre plynulý pohyb (lineárna interpolácia a smoothstep funkcia nestačí)

### Simulácia, časticový systém
- [X] (3b) Efektívna kolízia medzi objektami
- [X] (2b) Simulácia aspoň s dvoma silami s použitím vektorovej algebry
- [X] (2b) Simulácia pevných telies (rigid bodies) so správný výpočtom síl, kombinácie lineárneho a uhlového pohybu, a aplikácie krútiaceho momentu

### Práca s kamerou
- [X] (1b) Kamera s perspektívnou projekciou, správna zmena veľkosti okna a zobrazovacieho priestoru
- [X] (2b) Animovaná kamera pomocou kľúčových snímkov, ktoré obsahujú informáciu v danom čase kde sa kamera nachádza a kam sa pozerá, použitie inej ako lineárnej interpolácie

### Osvetlenie za pomoci viacerých svetelných zdrojov
- [X] (3b) Použitie všetkých typov zdrojov svetla s príslušnými parametrami: smerové, bodové, reflektor
- [X] (1b) Zmena pozície a orientácie zdrojov svetla a odtieňov farby osvetlenia
- [X] (1b) Správne kombinovať difúzne svetlo z 2 zdrojov svetla s difúznymi materiálmi
- [X] (2b) Správny Phongov osvetlovací model s viacerými (aspoň 3) zdrojmi svetla, pričom musí byť splnené: 
  - Správne tlmenie svetla na základe hĺbky
  - Použité aspoň tri zložky farby materiálu a tri zložky farby svetla
  - Správne kombinovať zložky farby materialu a zložky farby svetla
- [X] (1b) Implementácia Blinn-Phongovho modifikovaného osvetľovacieho modelu
- [ ] (1b) HDR zobrazenie s použitím mapovania tónov (tone mapping) a gama korekcie
- [ ] (1b) Tiene implementované jednoduchým spôsobom
- [X] (4b) Tiene implementované pomocou shadow-maps

### Post-processing. Zobrazenie scény s aplikovaným post-processing filtrom
- [X] (1b) Základný per-pixel filter napr. grayscale
- [X] (2b) konvolučný filter cez framebuffer
- [] (4b) pokročilý filter, napr. bloom cez framebuffer