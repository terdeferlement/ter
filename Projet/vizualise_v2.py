#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Visualisation Saint-Venant 1D avec Bathymétrie
Affiche la surface libre, le fond et la hauteur d'eau
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d import Axes3D

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
# GRAPHIQUE 1 : ÉVOLUTION DE LA VAGUE SUR LA PLAGE
# ================================================
print("Création du graphique d'évolution de la vague...")

n_instants = min(8, len(temps_uniques))
indices_temps = np.linspace(0, len(temps_uniques)-1, n_instants, dtype=int)

fig, ax = plt.subplots(figsize=(14, 7))

# Couleurs pour les différents instants
couleurs = plt.cm.Blues(np.linspace(0.3, 1, n_instants))

for i, idx in enumerate(indices_temps):
    t = temps_uniques[idx]
    indices = np.where(temps == t)[0]
    
    x_t = x[indices]
    h_t = h[indices]
    zb_t = zb[indices]
    H_t = H[indices]  # Surface libre
    
    # Tracer la surface libre (niveau de l'eau)
    ax.plot(x_t, H_t, color=couleurs[i], linewidth=2.5, 
            label=f't = {t:.2f} s', zorder=10-i)
    
    # Remplir la zone d'eau
    ax.fill_between(x_t, zb_t, H_t, color=couleurs[i], alpha=0.3, zorder=10-i)

# Tracer le fond (bathymétrie) en dernière position
indices_final = np.where(temps == temps_uniques[-1])[0]
x_final = x[indices_final]
zb_final = zb[indices_final]
ax.plot(x_final, zb_final, 'k-', linewidth=3, label='Fond (bathymétrie)', zorder=20)
ax.fill_between(x_final, -0.5, zb_final, color='saddlebrown', alpha=0.5, zorder=1)

ax.set_xlabel('Position x (m)', fontsize=13)
ax.set_ylabel('Altitude (m)', fontsize=13)
ax.set_title('Propagation de la vague sur la plage avec bathymétrie', fontsize=15, fontweight='bold')
ax.grid(True, alpha=0.3, linestyle='--')
ax.legend(fontsize=10, loc='upper left', ncol=2)
ax.set_ylim(-0.2, max(H.max(), zb.max()) + 0.3)

plt.tight_layout()
plt.savefig('images/vague_bathymetrie.png', dpi=200, bbox_inches='tight')
print("✓ Sauvegardé : vague_bathymetrie.png")
plt.show()

# ================================================
# GRAPHIQUE 2 : ÉTAT FINAL DÉTAILLÉ
# ================================================
print("\nCréation du graphique de l'état final...")

t_final = temps_uniques[-1]
indices_final = np.where(temps == t_final)[0]

x_final = x[indices_final]
h_final = h[indices_final]
u_final = u[indices_final]
zb_final = zb[indices_final]
H_final = H[indices_final]

fig, axes = plt.subplots(3, 1, figsize=(14, 10))

# Subplot 1 : Surface libre + Bathymétrie
axes[0].plot(x_final, H_final, 'b-', linewidth=2.5, label='Surface libre', zorder=10)
axes[0].plot(x_final, zb_final, 'k-', linewidth=2.5, label='Fond (bathymétrie)', zorder=5)
axes[0].fill_between(x_final, zb_final, H_final, color='cyan', alpha=0.4, label='Eau', zorder=3)
axes[0].fill_between(x_final, -0.5, zb_final, color='saddlebrown', alpha=0.4, label='Sol', zorder=1)
axes[0].set_ylabel('Altitude (m)', fontsize=12)
axes[0].set_title(f'État final à t = {t_final:.3f} s', fontsize=14, fontweight='bold')
axes[0].legend(fontsize=10, loc='upper left')
axes[0].grid(True, alpha=0.3)
axes[0].set_ylim(-0.2, max(H_final.max(), zb_final.max()) + 0.3)

# Subplot 2 : Hauteur d'eau
axes[1].plot(x_final, h_final, 'b-', linewidth=2)
axes[1].fill_between(x_final, 0, h_final, color='blue', alpha=0.3)
axes[1].set_ylabel('Hauteur h (m)', fontsize=12)
axes[1].set_title('Hauteur d\'eau', fontsize=13)
axes[1].grid(True, alpha=0.3)
axes[1].set_ylim(bottom=0)

# Subplot 3 : Vitesse
axes[2].plot(x_final, u_final, 'r-', linewidth=2)
axes[2].fill_between(x_final, 0, u_final, color='red', alpha=0.3)
axes[2].axhline(y=0, color='k', linestyle='--', alpha=0.5)
axes[2].set_xlabel('Position x (m)', fontsize=12)
axes[2].set_ylabel('Vitesse u (m/s)', fontsize=12)
axes[2].set_title('Vitesse de l\'eau', fontsize=13)
axes[2].grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('images/etat_final_complet.png', dpi=200, bbox_inches='tight')
print("✓ Sauvegardé : etat_final_complet.png")
plt.show()

# # ================================================
# # GRAPHIQUE 3 : DIAGRAMME ESPACE-TEMPS 3D
# # ================================================
# print("\nCréation du diagramme espace-temps 3D...")

# fig = plt.figure(figsize=(14, 9))
# ax = fig.add_subplot(111, projection='3d')

# # Surface libre dans le temps
# surf = ax.plot_trisurf(x, temps, H, cmap='viridis', alpha=0.8, 
#                        linewidth=0, antialiased=True, edgecolor='none')

# # Ajouter le fond (bathymétrie) à tous les temps
# for t in temps_uniques[::max(1, len(temps_uniques)//10)]:
#     indices = np.where(temps == t)[0]
#     ax.plot(x[indices], [t]*len(indices), zb[indices], 'k-', linewidth=1, alpha=0.3)

# ax.set_xlabel('Position x (m)', fontsize=12, labelpad=10)
# ax.set_ylabel('Temps t (s)', fontsize=12, labelpad=10)
# ax.set_zlabel('Altitude (m)', fontsize=12, labelpad=10)
# ax.set_title('Diagramme espace-temps : Surface libre et bathymétrie', 
#              fontsize=14, fontweight='bold', pad=20)

# # Barre de couleur
# cbar = fig.colorbar(surf, ax=ax, shrink=0.5, aspect=10)
# cbar.set_label('Surface libre (m)', fontsize=11)

# # Angle de vue optimal
# ax.view_init(elev=25, azim=45)

# plt.savefig('images/espace_temps_3D.png', dpi=200, bbox_inches='tight')
# print("✓ Sauvegardé : espace_temps_3D.png")
# plt.show()

# ================================================
# GRAPHIQUE 4 : ÉVOLUTION DE LA HAUTEUR D'EAU
# ================================================
print("\nCréation de l'évolution de la hauteur d'eau...")

fig, ax = plt.subplots(figsize=(14, 7))

couleurs_h = plt.cm.plasma(np.linspace(0.1, 0.9, n_instants))

for i, idx in enumerate(indices_temps):
    t = temps_uniques[idx]
    indices = np.where(temps == t)[0]
    
    x_t = x[indices]
    h_t = h[indices]
    
    ax.plot(x_t, h_t, color=couleurs_h[i], linewidth=2.5, 
            label=f't = {t:.2f} s', zorder=10-i)
    ax.fill_between(x_t, 0, h_t, color=couleurs_h[i], alpha=0.2, zorder=10-i)

ax.set_xlabel('Position x (m)', fontsize=13)
ax.set_ylabel('Hauteur d\'eau h (m)', fontsize=13)
ax.set_title('Évolution de la hauteur d\'eau au cours du temps', fontsize=15, fontweight='bold')
ax.grid(True, alpha=0.3, linestyle='--')
ax.legend(fontsize=10, loc='best', ncol=2)
ax.set_ylim(bottom=0)

plt.tight_layout()
plt.savefig('images/evolution_hauteur.png', dpi=200, bbox_inches='tight')
print("✓ Sauvegardé : evolution_hauteur.png")
plt.show()

# ================================================
# STATISTIQUES
# ================================================
print("\n" + "="*60)
print("STATISTIQUES DE LA SIMULATION")
print("="*60)

# Pour l'instant initial
indices_init = np.where(temps == temps_uniques[0])[0]
h_init_max = h[indices_init].max()
x_init_max = x[indices_init][np.argmax(h[indices_init])]

# Pour l'instant final
h_final_max = h_final.max()
x_final_max = x_final[np.argmax(h_final)]

print(f"\nInstant initial (t = {temps_uniques[0]:.3f} s):")
print(f"  • Hauteur max : {h_init_max:.4f} m")
print(f"  • Position crête : {x_init_max:.2f} m")

print(f"\nInstant final (t = {t_final:.3f} s):")
print(f"  • Hauteur max : {h_final_max:.4f} m")
print(f"  • Position crête : {x_final_max:.2f} m")
print(f"  • Vitesse max : {abs(u_final).max():.4f} m/s")

print(f"\nDéplacement de la crête : {x_final_max - x_init_max:.2f} m")
print(f"Vitesse moyenne de propagation : {(x_final_max - x_init_max)/t_final:.2f} m/s")

print(f"\nBathymétrie:")
print(f"  • Altitude min : {zb_final.min():.2f} m")
print(f"  • Altitude max : {zb_final.max():.2f} m")

print("\n" + "="*60)
print("VISUALISATION TERMINÉE !")
print("="*60)