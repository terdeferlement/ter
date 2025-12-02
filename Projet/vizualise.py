#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Script SIMPLE de visualisation pour Saint-Venant 1D
"""

import numpy as np
import matplotlib.pyplot as plt

# ================================================
# Charger les données du fichier
# ================================================
print("Chargement des données depuis solution.txt...")
data = np.loadtxt('solution.txt')

# Colonnes : temps, x, h, u
temps = data[:, 0]
x = data[:, 1]
h = data[:, 2]
u = data[:, 3]

print(f"Nombre de points : {len(temps)}")
print(f"Temps min : {temps.min():.4f} s")
print(f"Temps max : {temps.max():.4f} s")
print()

# ================================================
# Trouver les différents instants sauvegardés
# ================================================
temps_uniques = np.unique(temps)
print(f"Nombre d'instants sauvegardés : {len(temps_uniques)}")

# ================================================
# GRAPHIQUE 1 : État final
# ================================================
print("Création du graphique de l'état final...")

# Prendre le dernier instant
t_final = temps_uniques[-1]
indices_final = np.where(temps == t_final)[0]

x_final = x[indices_final]
h_final = h[indices_final]
u_final = u[indices_final]

# Créer la figure
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

# Graphique de la hauteur
ax1.plot(x_final, h_final, 'b-', linewidth=2)
ax1.set_xlabel('Position x (m)', fontsize=12)
ax1.set_ylabel('Hauteur h (m)', fontsize=12)
ax1.set_title(f'Hauteur d\'eau à t = {t_final:.3f} s', fontsize=14)
ax1.grid(True, alpha=0.3)
ax1.set_ylim(bottom=0)

# Graphique de la vitesse
ax2.plot(x_final, u_final, 'r-', linewidth=2)
ax2.set_xlabel('Position x (m)', fontsize=12)
ax2.set_ylabel('Vitesse u (m/s)', fontsize=12)
ax2.set_title(f'Vitesse à t = {t_final:.3f} s', fontsize=14)
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('images/resultat_final.png', dpi=150)
print("Sauvegardé : resultat_final.png")
# plt.show()

# ================================================
# GRAPHIQUE 2 : Évolution temporelle
# ================================================
print()
print("Création du graphique d'évolution temporelle...")

# Prendre 5 instants différents
n_instants = min(5, len(temps_uniques))
indices_temps = np.linspace(0, len(temps_uniques)-1, n_instants, dtype=int)

plt.figure(figsize=(10, 6))

# Palette de couleurs
couleurs = plt.cm.viridis(np.linspace(0, 1, n_instants))

for i, idx in enumerate(indices_temps):
    t = temps_uniques[idx]
    indices = np.where(temps == t)[0]
    
    x_t = x[indices]
    h_t = h[indices]
    
    plt.plot(x_t, h_t, color=couleurs[i], linewidth=2, 
             label=f't = {t:.3f} s')

plt.xlabel('Position x (m)', fontsize=12)
plt.ylabel('Hauteur h (m)', fontsize=12)
plt.title('Évolution de la hauteur d\'eau', fontsize=14)
plt.grid(True, alpha=0.3)
plt.ylim(bottom=0)
plt.legend(fontsize=10)
plt.tight_layout()
plt.savefig('images/evolution.png', dpi=150)
print("Sauvegardé : evolution.png")
# plt.show()

# ================================================
# GRAPHIQUE 3 : Diagramme espace-temps (3D)
# ================================================
print()
print("Création du diagramme espace-temps 3D...")

from mpl_toolkits.mplot3d import Axes3D

fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

# Afficher tous les points
scatter = ax.scatter(x, temps, h, c=h, cmap='viridis', s=1)

ax.set_xlabel('Position x (m)', fontsize=12)
ax.set_ylabel('Temps t (s)', fontsize=12)
ax.set_zlabel('Hauteur h (m)', fontsize=12)
ax.set_title('Diagramme espace-temps', fontsize=14)

# Barre de couleur
cbar = fig.colorbar(scatter, ax=ax, shrink=0.5)
cbar.set_label('Hauteur (m)', fontsize=10)

plt.savefig('images/espace_temps.png', dpi=150)
print("Sauvegardé : espace_temps.png")
# plt.show()

print()
print("=" * 50)
print("Visualisation terminée !")
print("=" * 50)