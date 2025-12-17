import pandas as pd
import plotly.graph_objects as go
import numpy as np

def generer_graphique_interactif_leger():
    print("Chargement des données...")
    
    # 1. Lire le fichier
    columns = ['t', 'x', 'h', 'u', 'zb', 'H']
    try:
        df = pd.read_csv('solution.txt', sep=' ', header=None, names=columns)
    except FileNotFoundError:
        print("Erreur : Fichier solution.txt introuvable.")
        return

    print(f"Taille originale : {len(df)} lignes.")
    
    # =========================================================
    # OPTIMISATION : ON RÉDUIT LA TAILLE DES DONNÉES
    # =========================================================
    # Facteur de réduction (ex: 10 = on garde 1 ligne sur 10)
    # Plus le fichier est gros, plus il faut augmenter ce chiffre (10, 20, 50...)
    facteur_reduction = 10 
    
    if len(df) > 100000:
        print(f"Le fichier est gros ! On ne garde qu'une ligne sur {facteur_reduction}...")
        df = df.iloc[::facteur_reduction, :]
    
    print("Transformation des données (Pivot)...")
    
    # Matrice de la surface (H)
    z_data = df.pivot_table(index='t', columns='x', values='H')
    
    # Matrice du fond (zb)
    zb_profile = df.pivot_table(index='t', columns='x', values='zb').iloc[0]
    zb_matrix = pd.DataFrame([zb_profile.values] * len(z_data), index=z_data.index, columns=z_data.columns)

    x_axis = z_data.columns
    t_axis = z_data.index

    print("Génération du graphique 3D...")

    fig = go.Figure()

    # Eau
    fig.add_trace(go.Surface(
        z=z_data.values, x=x_axis, y=t_axis,
        colorscale='Viridis', opacity=0.9, name='Eau'
    ))

    # Fond
    fig.add_trace(go.Surface(
        z=zb_matrix.values, x=x_axis, y=t_axis,
        colorscale=[[0, 'tan'], [1, 'saddlebrown']],
        showscale=False, opacity=1.0, name='Fond'
    ))

    fig.update_layout(
        title='Simulation 3D (Allégée)',
        scene=dict(
            xaxis_title='X (m)',
            yaxis_title='Temps (s)',
            zaxis_title='Z (m)',
        ),
        autosize=True,
        margin=dict(l=0, r=0, b=0, t=50)
    )

    nom_fichier = "simulation_3d_legere.html"
    fig.write_html(nom_fichier)
    print(f"Fini ! Ouvre '{nom_fichier}'")

if __name__ == "__main__":
    generer_graphique_interactif_leger()