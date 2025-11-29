# ============================================================
#  levee_visualisation.gnu
#  Génération d'images PNG pour chaque bloc (surface libre + vitesse)
#  Version 100% compatible Gnuplot
# ============================================================
reset
set datafile commentschars "#"
set datafile separator whitespace
set grid
set style line 1 lw 2

# --- Fichier de données généré par ton code C++ ---
datafile = "solution_godunov.dat"

# --- Détection automatique du nombre de blocs ---
stats datafile nooutput
nb_blocs = STATS_blocks

print sprintf("Nombre de blocs détectés : %d", nb_blocs)
print "Génération des images PNG en cours..."

# --- Bathymétrie (fond) : constante ou fichier ---
zb0 = 0.0         # fond plat (modifiable)
# Si tu as un fichier de bathymétrie "bathy.dat" (x, zb), décommente la ligne suivante :
# bathyfile = "bathy.dat"

# ============================================================
# Boucle sur tous les blocs
# ============================================================
do for [i=0:nb_blocs-1] {
    
    # --- Fichier de sortie pour ce bloc ---
    outfile = sprintf("bloc_%03d.png", i)
    
    # --- Configuration du terminal PNG (DOIT être avant set output) ---
    set term pngcairo size 1200,800 enhanced font "Arial,12"
    set output outfile
    
    # --- Layout multiplot : 2 graphiques verticaux ---
    set multiplot layout 2,1 title sprintf("Simulation - Bloc %d/%d", i, nb_blocs-1)
    
    # ============================================================
    # 1️⃣ GRAPHIQUE DU HAUT : Surface libre et bathymétrie
    # ============================================================
    set title "Profil de surface libre et bathymétrie"
    set xlabel "x (m)"
    set ylabel "Élévation (m)"
    set grid
    set key inside top right
    set autoscale
    
    # Cas avec bathymétrie à partir d'un fichier
    if (exists("bathyfile")) {
        plot datafile index i using 1:($2 + 0) with lines lw 2 lc rgb "blue" title "Surface libre (h + zb)", \
             bathyfile using 1:2 with lines lw 2 lc rgb "brown" title "Bathymétrie zb"
    } else {
        plot datafile index i using 1:($2 + zb0) with lines lw 2 lc rgb "blue" title "Surface libre (h + zb)", \
             datafile index i using 1:(zb0) with lines lw 2 lc rgb "brown" title "Fond zb (plat)"
    }
    
    # ============================================================
    # 2️⃣ GRAPHIQUE DU BAS : Vitesse u(x)
    # ============================================================
    set title "Champ de vitesse moyenne u(x)"
    set xlabel "x (m)"
    set ylabel "Vitesse (m/s)"
    set key inside top right
    set autoscale
    
    plot datafile index i using 1:3 with lines lw 2 lc rgb "red" title "Vitesse u(x)"
    
    unset multiplot
    set output
    
    print sprintf("✅ Généré : %s", outfile)
}

# --- Retour au terminal interactif ---
set term qt

print ""
print sprintf("🎉 Terminé ! %d images PNG générées.", nb_blocs)
print "Fichiers : bloc_000.png, bloc_001.png, ..., bloc_%03d.png", nb_blocs-1