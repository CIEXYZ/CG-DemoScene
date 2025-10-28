# Disco club
### Juraj Majernik; Valeriia Petrishka

---
## Tasks

### 3D objekty
- [X] (2b) Objekty ako unikátne 3D polygonálne modely 
- [ ] (2b) Unikátne mapovanie textúr pomocou UV koordinátov na 3D objekt
- [ ] (2b) Efektívne inštancovanie 3D objektu: zobrazenie aspoň 5000 inštancií za použitia OpenGL inštancovania
- [ ] (2b) Priesvitné objekty
- [ ] (1b) Objekt s využitím hrbolatej textúry (normal mapping)

### Scéna
- [ ] (2b) Procedurálne generovaná scéna
- [ ] (4b) Hierarchická reprezentácia scény
- [ ] (2b) Využitie techniky mapovania na kocku (cube mapping)

### Animované objekty
- [ ] (2b) Procedurálna animácia
- [ ] (3b) Animácia na základe dát uložených v kľúčových snímkoch a interpolácie
- [ ] (1b) Animácia riadená komplexnými animačnými krivkami pre plynulý pohyb (lineárna interpolácia a smoothstep funkcia nestačí)

### Simulácia, časticový systém
- [ ] (3b) Efektívna kolízia medzi objektami
- [ ] (2b) Simulácia aspoň s dvoma silami s použitím vektorovej algebry
- [ ] (2b) Simulácia pevných telies (rigid bodies) so správný výpočtom síl, kombinácie lineárneho a uhlového pohybu, a aplikácie krútiaceho momentu

### Práca s kamerou
- [ ] (1b) Kamera s perspektívnou projekciou, správna zmena veľkosti okna a zobrazovacieho priestoru
- [ ] (2b) Animovaná kamera pomocou kľúčových snímkov, ktoré obsahujú informáciu v danom čase kde sa kamera nachádza a kam sa pozerá, použitie inej ako lineárnej interpolácie

### Osvetlenie za pomoci viacerých svetelných zdrojov
- [ ] (3b) Použitie všetkých typov zdrojov svetla s príslušnými parametrami: smerové, bodové, reflektor
- [ ] (1b) Zmena pozície a orientácie zdrojov svetla a odtieňov farby osvetlenia
- [ ] (1b) Správne kombinovať difúzne svetlo z 2 zdrojov svetla s difúznymi materiálmi
- [ ] (2b) Správny Phongov osvetlovací model s viacerými (aspoň 3) zdrojmi svetla, pričom musí byť splnené: 
  - Správne tlmenie svetla na základe hĺbky
  - Použité aspoň tri zložky farby materiálu a tri zložky farby svetla
  - Správne kombinovať zložky farby materialu a zložky farby svetla
- [ ] (1b) Implementácia Blinn-Phongovho modifikovaného osvetľovacieho modelu
- [ ] (1b) HDR zobrazenie s použitím mapovania tónov (tone mapping) a gama korekcie
- [ ] (1b) Tiene implementované jednoduchým spôsobom
- [ ] (4b) Tiene implementované pomocou shadow-maps

### Post-processing. Zobrazenie scény s aplikovaným post-processing filtrom
- [ ] (1b) Základný per-pixel filter napr. grayscale
- [ ] (2b) konvolučný filter cez framebuffer
- [ ] (4b) pokročilý filter, napr. bloom cez framebuffer