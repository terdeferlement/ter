#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Visualisation Saint-Venant 1D - Évolution temporelle et Animation
Génère :
1. Évolution de la hauteur maximale en fonction du temps
2. Évolution de la vitesse maximale en fonction du temps
3. Animation GIF de la propagation de la vague
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter
import os

# Créer le dossier images s'il n'existe pas
if not os.path.exists('images'):
    os.makedirs('images')
    print("Dossier 'images' créé.")

# ================================================
# CHARGEMENT DES DONNÉES
# ================================================
print("Chargement des données depuis solution.txt...")
data = np.loadtxt('solution.txt')

# Colonnes : temps, x, h, u, zb, H
temps = data[:, 0]
x = data[:, 1]
h = data[:, 2]      # Hauteur d'eau
u = data[:, 3]      # Vitesse
zb = data[:, 4]     # Bathymétrie (altitude du fond)
H = data[:, 5]      # Surface libre (h + zb)

print(f"Nombre de points : {len(temps)}")
print(f"Temps min : {temps.min():.4f} s")
print(f"Temps max : {temps.max():.4f} s")

# Instants sauvegardés
temps_uniques = np.unique(temps)
print(f"Nombre d'instants : {len(temps_uniques)}")
print()

# ================================================
# GRAPHIQUE 1 : ÉVOLUTION DE LA HAUTEUR MAXIMALE
# ================================================
print("Création du graphique d'évolution de la hauteur maximale...")

# Chercher le maximum de la SURFACE LIBRE (H = h + zb) là où il y a de l'eau
h_max_evolution = []
H_max_evolution = []  # Surface libre maximale
for t in temps_uniques:
    indices = np.where(temps == t)[0]
    h_t = h[indices]
    H_t = H[indices]
    
    # Filtrer pour ne regarder que là où il y a de l'eau (h > seuil)
    mask_eau = h_t > 1e-4
    if np.any(mask_eau):
        h_max_evolution.append(h_t[mask_eau].max())
        H_max_evolution.append(H_t[mask_eau].max())
    else:
        h_max_evolution.append(0)
        H_max_evolution.append(0)

fig, ax = plt.subplots(figsize=(12, 6))
ax.plot(temps_uniques, H_max_evolution, 'b-', linewidth=2.5, marker='o', 
        markersize=4, markerfacecolor='white', markeredgewidth=1.5, label='Surface libre maximale')
ax.plot(temps_uniques, h_max_evolution, 'c--', linewidth=2, marker='o', 
        markersize=3, alpha=0.7, label='Hauteur d\'eau maximale')
ax.set_xlabel('Temps (s)', fontsize=13)
ax.set_ylabel('Altitude / Hauteur maximale (m)', fontsize=13)
ax.set_title('Évolution de la hauteur maximale de la vague au cours du temps', 
             fontsize=15, fontweight='bold')
ax.grid(True, alpha=0.3, linestyle='--')
ax.set_ylim(bottom=0)
ax.legend(fontsize=10, loc='best')

# Ajout de statistiques sur le graphique
H_max_init = H_max_evolution[0]
H_max_final = H_max_evolution[-1]
h_max_init = h_max_evolution[0]
h_max_final = h_max_evolution[-1]
variation_H = ((H_max_final - H_max_init) / H_max_init) * 100 if H_max_init > 0 else 0
variation_h = ((h_max_final - h_max_init) / h_max_init) * 100 if h_max_init > 0 else 0
textstr = f'Surface libre:\n'
textstr += f'  Initiale: {H_max_init:.4f} m\n'
textstr += f'  Finale: {H_max_final:.4f} m\n'
textstr += f'  Variation: {variation_H:.2f}%\n\n'
textstr += f'Hauteur d\'eau:\n'
textstr += f'  Initiale: {h_max_init:.4f} m\n'
textstr += f'  Finale: {h_max_final:.4f} m\n'
textstr += f'  Variation: {variation_h:.2f}%'
ax.text(0.02, 0.98, textstr, transform=ax.transAxes, fontsize=9,
        verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

# plt.tight_layout()
# plt.savefig('images/hauteur_max_evolution.png', dpi=200, bbox_inches='tight')
# print("✓ Sauvegardé : hauteur_max_evolution.png")
# plt.show()

# ================================================
# GRAPHIQUE 2 : ÉVOLUTION DE LA VITESSE MAXIMALE
# ================================================
print("\nCréation du graphique d'évolution de la vitesse maximale...")

# Chercher la vitesse au point où H est maximal (au sommet de la vague)
u_max_evolution = []
u_au_pic_evolution = []  # Vitesse au sommet de la vague
for t in temps_uniques:
    indices = np.where(temps == t)[0]
    h_t = h[indices]
    u_t = u[indices]
    H_t = H[indices]
    
    # Filtrer pour ne regarder que là où il y a de l'eau
    mask_eau = h_t > 1e-4
    if np.any(mask_eau):
        # Vitesse maximale absolue
        u_max_evolution.append(np.abs(u_t[mask_eau]).max())
        
        # Vitesse au pic de la vague (là où H est maximal)
        idx_pic = np.argmax(H_t)
        u_au_pic_evolution.append(np.abs(u_t[idx_pic]))
    else:
        u_max_evolution.append(0)
        u_au_pic_evolution.append(0)

fig, ax = plt.subplots(figsize=(12, 6))
ax.plot(temps_uniques, u_au_pic_evolution, 'r-', linewidth=2.5, marker='s', 
        markersize=4, markerfacecolor='white', markeredgewidth=1.5, label='Vitesse au sommet de la vague')
ax.plot(temps_uniques, u_max_evolution, 'm--', linewidth=2, marker='o', 
        markersize=3, alpha=0.7, label='Vitesse maximale absolue')
ax.set_xlabel('Temps (s)', fontsize=13)
ax.set_ylabel('Vitesse (m/s)', fontsize=13)
ax.set_title('Évolution de la vitesse maximale de la vague au cours du temps', 
             fontsize=15, fontweight='bold')
ax.grid(True, alpha=0.3, linestyle='--')
ax.set_ylim(bottom=0)
ax.legend(fontsize=10, loc='best')

# Ajout de statistiques sur le graphique
u_pic_init = u_au_pic_evolution[0]
u_pic_final = u_au_pic_evolution[-1]
u_pic_moyenne = np.mean(u_au_pic_evolution)
u_max_init = u_max_evolution[0]
u_max_final = u_max_evolution[-1]
textstr = f'Vitesse au sommet:\n'
textstr += f'  Initiale: {u_pic_init:.4f} m/s\n'
textstr += f'  Finale: {u_pic_final:.4f} m/s\n'
textstr += f'  Moyenne: {u_pic_moyenne:.4f} m/s\n\n'
textstr += f'Vitesse max absolue:\n'
textstr += f'  Initiale: {u_max_init:.4f} m/s\n'
textstr += f'  Finale: {u_max_final:.4f} m/s'
ax.text(0.02, 0.98, textstr, transform=ax.transAxes, fontsize=9,
        verticalalignment='top', bbox=dict(boxstyle='round', facecolor='lightcoral', alpha=0.5))

# plt.tight_layout()
# plt.savefig('images/vitesse_max_evolution.png', dpi=200, bbox_inches='tight')
# print("✓ Sauvegardé : vitesse_max_evolution.png")
# plt.show()

# ================================================
# GRAPHIQUE 3 : ÉVOLUTION COMBINÉE
# ================================================
print("\nCréation du graphique combiné hauteur et vitesse...")

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10))

# Hauteur maximale (surface libre)
ax1.plot(temps_uniques, H_max_evolution, 'b-', linewidth=2.5, marker='o', 
         markersize=4, label='Surface libre maximale')
ax1.plot(temps_uniques, h_max_evolution, 'c--', linewidth=2, marker='o', 
         markersize=3, alpha=0.7, label='Hauteur d\'eau maximale')
ax1.set_ylabel('Altitude / Hauteur (m)', fontsize=12, color='b')
ax1.set_title('Évolution des caractéristiques de la vague', fontsize=15, fontweight='bold')
ax1.grid(True, alpha=0.3, linestyle='--')
ax1.tick_params(axis='y', labelcolor='b')
ax1.set_ylim(bottom=0)
ax1.legend(loc='upper right', fontsize=10)

# Vitesse au sommet
ax2.plot(temps_uniques, u_au_pic_evolution, 'r-', linewidth=2.5, marker='s', 
         markersize=4, label='Vitesse au sommet')
ax2.plot(temps_uniques, u_max_evolution, 'm--', linewidth=2, marker='o', 
         markersize=3, alpha=0.7, label='Vitesse max absolue')
ax2.set_xlabel('Temps (s)', fontsize=12)
ax2.set_ylabel('Vitesse (m/s)', fontsize=12, color='r')
ax2.grid(True, alpha=0.3, linestyle='--')
ax2.tick_params(axis='y', labelcolor='r')
ax2.set_ylim(bottom=0)
ax2.legend(loc='upper right', fontsize=10)

# plt.tight_layout()
# plt.savefig('images/evolution_combinee.png', dpi=200, bbox_inches='tight')
# print("✓ Sauvegardé : evolution_combinee.png")
# plt.show()

# ================================================
# ANIMATION GIF : PROPAGATION DE LA VAGUE
# ================================================
print("\nCréation de l'animation GIF de la propagation de la vague...")
print("(Cela peut prendre quelques instants...)")

# Préparation des données pour l'animation
fig, ax = plt.subplots(figsize=(14, 7))

# Limites fixes pour l'animation
x_min, x_max = x.min(), x.max()
y_min = -0.2
y_max = max(H.max(), zb.max()) + 0.3

# Fonction d'initialisation
def init():
    ax.clear()
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)
    ax.set_xlabel('Position x (m)', fontsize=13)
    ax.set_ylabel('Altitude (m)', fontsize=13)
    ax.grid(True, alpha=0.3, linestyle='--')
    return []

# Fonction de mise à jour pour chaque frame
def update(frame):
    ax.clear()
    
    t = temps_uniques[frame]
    indices = np.where(temps == t)[0]
    
    x_t = x[indices]
    h_t = h[indices]
    zb_t = zb[indices]
    H_t = H[indices]
    
    # Tracer le fond (bathymétrie)
    ax.fill_between(x_t, -0.5, zb_t, color='saddlebrown', alpha=0.5, zorder=1)
    ax.plot(x_t, zb_t, 'k-', linewidth=3, label='Fond (bathymétrie)', zorder=5)
    
    # Tracer la surface libre et remplir la zone d'eau
    ax.fill_between(x_t, zb_t, H_t, color='cyan', alpha=0.6, zorder=3, label='Eau')
    ax.plot(x_t, H_t, 'b-', linewidth=2.5, label='Surface libre', zorder=10)
    
    # Configuration du graphique
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)
    ax.set_xlabel('Position x (m)', fontsize=13)
    ax.set_ylabel('Altitude (m)', fontsize=13)
    ax.set_title(f'Propagation de la vague sur la plage - t = {t:.3f} s', 
                 fontsize=15, fontweight='bold')
    ax.grid(True, alpha=0.3, linestyle='--')
    ax.legend(fontsize=11, loc='upper left')
    
    # Ajouter les informations sur la vague
    u_t = u[indices]
    
    # Trouver le sommet de la vague (où H est maximal)
    idx_pic = np.argmax(H_t)
    H_max_t = H_t[idx_pic]  # Surface libre maximale
    h_max_t = h_t[idx_pic]  # Hauteur d'eau au sommet
    x_max_t = x_t[idx_pic]  # Position du sommet
    u_pic_t = u_t[idx_pic]  # Vitesse au sommet
    
    textstr = f'Surface libre max: {H_max_t:.4f} m\n'
    textstr += f'Hauteur d\'eau: {h_max_t:.4f} m\n'
    textstr += f'Position crête: {x_max_t:.2f} m\n'
    textstr += f'Vitesse au sommet: {abs(u_pic_t):.4f} m/s'
    ax.text(0.98, 0.98, textstr, transform=ax.transAxes, fontsize=10,
            verticalalignment='top', horizontalalignment='right',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    return []

# Créer l'animation (avec tous les instants ou un sous-ensemble)
# Pour accélérer, on peut prendre 1 frame sur N
skip_frames = max(1, len(temps_uniques) // 100)  # Maximum 100 frames
frames_to_use = range(0, len(temps_uniques), skip_frames)

print(f"Nombre de frames dans l'animation : {len(frames_to_use)}")

anim = FuncAnimation(fig, update, frames=frames_to_use, init_func=init, 
                     blit=False, repeat=True, interval=100)

# Sauvegarder en GIF
writer = PillowWriter(fps=10)
anim.save('images/vague_animation.gif', writer=writer, dpi=150)
print("✓ Sauvegardé : vague_animation.gif")
plt.close()

# ================================================
# STATISTIQUES FINALES
# ================================================
print("\n" + "="*60)
print("STATISTIQUES DE LA SIMULATION")
print("="*60)

print(f"\nSurface libre maximale:")
print(f"  • Initiale : {H_max_evolution[0]:.4f} m")
print(f"  • Finale : {H_max_evolution[-1]:.4f} m")
print(f"  • Maximum global : {max(H_max_evolution):.4f} m")
print(f"  • Minimum global : {min(H_max_evolution):.4f} m")

print(f"\nHauteur d'eau maximale:")
print(f"  • Initiale : {h_max_evolution[0]:.4f} m")
print(f"  • Finale : {h_max_evolution[-1]:.4f} m")
print(f"  • Maximum global : {max(h_max_evolution):.4f} m")
print(f"  • Minimum global : {min(h_max_evolution):.4f} m")

print(f"\nVitesse au sommet de la vague:")
print(f"  • Initiale : {u_au_pic_evolution[0]:.4f} m/s")
print(f"  • Finale : {u_au_pic_evolution[-1]:.4f} m/s")
print(f"  • Maximum global : {max(u_au_pic_evolution):.4f} m/s")
print(f"  • Moyenne : {np.mean(u_au_pic_evolution):.4f} m/s")

print(f"\nVitesse maximale absolue:")
print(f"  • Initiale : {u_max_evolution[0]:.4f} m/s")
print(f"  • Finale : {u_max_evolution[-1]:.4f} m/s")
print(f"  • Maximum global : {max(u_max_evolution):.4f} m/s")

print(f"\nDurée de la simulation : {temps_uniques[-1]:.3f} s")
print(f"Nombre d'instants sauvegardés : {len(temps_uniques)}")

print("\n" + "="*60)
print("GÉNÉRATION DES GRAPHIQUES TERMINÉE !")
print("="*60)
print("\nFichiers générés dans le dossier 'images/':")
print("  1. hauteur_max_evolution.png")
print("  2. vitesse_max_evolution.png")
print("  3. evolution_combinee.png")
print("  4. vague_animation.gif")
print("="*60)