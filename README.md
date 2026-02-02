# 2526_projet1a_pcbot1
PCBot

## Memo git

Pensez à ajouter votre clé ssh à votre compte github.

### Récupérer le projet

```bash
git clone git@github.com:rahim974/2526_projet1a_pcbot1.git
```

### Récupérer la dernière version

```bash
git pull
```

### Vérifier vos modifications

```bash
git status
```

### Pousser vos modifications

```bash
git add .   # À la place du ".", on peut mettre le chemin des fichiers à ajouter
git commit -m "Message (pour indiquer la mise à jour faite)"
git push
```


# Test de contribution
## Modification par Bilal
> "La configuration SSH et Git est opérationnelle."

---
*Dernière mise à jour le : 2 février 2026*

 ### choix des composants



# 2/02/2026

Le robot donne ses coordonnées et ce qu'il y a devant lui (distances aux obstacles).

## Cahier des charges

- Robot doit rouler 
- Connaitre son environnement (distances aux obstacles) 
- Connaitre sa vitesse et sa position
- Pouvoir le charger à une prise 
- Communiquer en continu via le module nrF24L01
- Communiquer avec une station de base (angle, distance)

## Matériels

- Capteur TOF (repérer ce qu'il y a devant)
- nRF24N https://passionelectronique.fr/tutorial-nrf24l01/
- Batterie 18650 Samsung (en USB) https://www.nkon.nl/fr/samsung-inr18650-35e-3400mah-8a.html?gad_source=1&gad_campaignid=21554454169&gclid=EAIaIQobChMI94uM95C7kgMVLWlBAh1DugIAEAQYASABEgIlePD_BwE
- Moteur continue https://www.gotronic.fr/art-motoreducteur-maqueen-v5-48529.htm?srsltid=AfmBOoo6Rrt8KTnTSC0lMwvMVoNbsPc9MDphmUr6xJ-IcqWVbKeVPGag
- Leds 
- Bouton d'alimentation
- Prise secteur https://www.ti.com/lit/ds/symlink/bq25896.pdf?ts=1769962316849
- accéléromètre https://www.st.com/en/mems-and-sensors/lsm6dsox.html
- drivers moteur https://www.ti.com/lit/ds/symlink/drv8411a.pdf?ts=1770046834572&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FDRV8411A%252Fpart-details%252FDRV8411APWPR
Ne pas utiliser des librairies déjà existantes sauf si très ressemblant.

## Rôles 

- Schéma architectural sur draw.io 
- Essais capteur TOF

Essais capteur TOF 

I2C entre moteur et accélérateur, driver de batterie

Outil : draw.io 























