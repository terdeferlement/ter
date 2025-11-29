#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Script de visualisation pour Saint-Venant 1D
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d import Axes3D

def load_data(filename):
    """Charge les données depuis le fichier solution"""
    data = np.loadtxt(filename)
    return data

def plot_final_state(filename='solution.txt'):
    """Affiche l'état final de la simulation"""
    data = load_data(filename)
    
    # Extraire les données uniques du dernier temps
    times = np.unique(data[:, 0])
    t_final = times[-1]
    
    # Filtrer pour le dernier temps
    final_data = data[data[:, 0] == t_final]
    x = final_data[:, 1]
    h = final_data[:, 2]
    u = final_data[:, 3]
    
    # Créer la figure
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    
    # Hauteur d'eau
    ax1.plot(x, h, 'b-', linewidth=2, label='h(x)')
    ax1.set_xlabel('x (m)', fontsize=12)
    ax1.set_ylabel('Hauteur h (m)', fontsize=12)
    ax1.set_title(f'Hauteur d\'eau à t = {t_final:.3f} s', fontsize=14)
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # Vitesse
    ax2.plot(x, u, 'r-', linewidth=2, label='u(x)')
    ax2.set_xlabel('x (m)', fontsize=12)
    ax2.set_ylabel('Vitesse u (m/s)', fontsize=12)
    ax2.set_title(f'Vitesse à t = {t_final:.3f} s', fontsize=14)
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    
    plt.tight_layout()
    plt.savefig('saint_venant_final.png', dpi=150)
    print("Figure sauvegardée : saint_venant_final.png")
    plt.show()

def plot_spacetime(filename='solution.txt'):
    """Affiche le diagramme espace-temps"""
    data = load_data(filename)
    
    t = data[:, 0]
    x = data[:, 1]
    h = data[:, 2]
    
    fig = plt.figure(figsize=(12, 6))
    ax = fig.add_subplot(111, projection='3d')
    
    # Scatter plot
    scatter = ax.scatter(x, t, h, c=h, cmap='viridis', s=1)
    
    ax.set_xlabel('Position x (m)', fontsize=12)
    ax.set_ylabel('Temps t (s)', fontsize=12)
    ax.set_zlabel('Hauteur h (m)', fontsize=12)
    ax.set_title('Diagramme espace-temps de h(x,t)', fontsize=14)
    
    cbar = fig.colorbar(scatter, ax=ax, shrink=0.5)
    cbar.set_label('Hauteur (m)', fontsize=10)
    
    plt.savefig('saint_venant_spacetime.png', dpi=150)
    print("Figure sauvegardée : saint_venant_spacetime.png")
    plt.show()

def create_animation(filename='solution.txt', output='animation.gif'):
    """Crée une animation de l'évolution temporelle"""
    data = load_data(filename)
    
    times = np.unique(data[:, 0])
    
    # Préparer la figure
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Limites des axes
    x_all = data[:, 1]
    h_all = data[:, 2]
    ax.set_xlim(x_all.min(), x_all.max())
    ax.set_ylim(0, h_all.max() * 1.1)
    
    line, = ax.plot([], [], 'b-', linewidth=2)
    time_text = ax.text(0.02, 0.95, '', transform=ax.transAxes, 
                        fontsize=12, verticalalignment='top')
    
    ax.set_xlabel('x (m)', fontsize=12)
    ax.set_ylabel('h (m)', fontsize=12)
    ax.set_title('Évolution de la hauteur d\'eau', fontsize=14)
    ax.grid(True, alpha=0.3)
    
    def init():
        line.set_data([], [])
        time_text.set_text('')
        return line, time_text
    
    def animate(i):
        t = times[i]
        frame_data = data[data[:, 0] == t]
        x = frame_data[:, 1]
        h = frame_data[:, 2]
        
        line.set_data(x, h)
        time_text.set_text(f't = {t:.4f} s')
        return line, time_text
    
    # Créer l'animation (prendre 1 frame sur 10 pour accélérer)
    frames = range(0, len(times), max(1, len(times)//100))
    anim = FuncAnimation(fig, animate, init_func=init, 
                        frames=frames, interval=50, blit=True)
    
    # Sauvegarder
    anim.save(output, writer='pillow', fps=20)
    print(f"Animation sauvegardée : {output}")
    plt.close()

def plot_comparison(filename='solution.txt'):
    """Compare plusieurs instants"""
    data = load_data(filename)
    times = np.unique(data[:, 0])
    
    # Sélectionner quelques instants
    n_snapshots = 5
    selected_times = [times[i] for i in np.linspace(0, len(times)-1, n_snapshots, dtype=int)]
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    colors = plt.cm.viridis(np.linspace(0, 1, n_snapshots))
    
    for i, t in enumerate(selected_times):
        frame_data = data[data[:, 0] == t]
        x = frame_data[:, 1]
        h = frame_data[:, 2]
        ax.plot(x, h, color=colors[i], linewidth=2, label=f't = {t:.3f} s')
    
    ax.set_xlabel('x (m)', fontsize=12)
    ax.set_ylabel('h (m)', fontsize=12)
    ax.set_title('Évolution de la hauteur d\'eau', fontsize=14)
    ax.grid(True, alpha=0.3)
    ax.legend(fontsize=10)
    
    plt.tight_layout()
    plt.savefig('saint_venant_comparison.png', dpi=150)
    print("Figure sauvegardée : saint_venant_comparison.png")
    plt.show()

if __name__ == '__main__':
    import sys
    
    print("=" * 50)
    print("Visualisation Saint-Venant 1D")
    print("=" * 50)
    print()
    
    filename = 'solution.txt' if len(sys.argv) < 2 else sys.argv[1]
    
    try:
        print(f"Chargement des données depuis : {filename}")
        print()
        
        print("1. Tracé de l'état final...")
        plot_final_state(filename)
        
        print()
        print("2. Diagramme espace-temps...")
        plot_spacetime(filename)
        
        print()
        print("3. Comparaison temporelle...")
        plot_comparison(filename)
        
        print()
        choice = input("Créer une animation ? (o/n) : ")
        if choice.lower() == 'o':
            print("Création de l'animation (peut prendre quelques secondes)...")
            create_animation(filename)
        
        print()
        print("Terminé !")
        
    except FileNotFoundError:
        print(f"Erreur : fichier {filename} introuvable.")
        print("Lancez d'abord la simulation C++.")
    except Exception as e:
        print(f"Erreur : {e}")