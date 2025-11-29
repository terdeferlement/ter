# ============================================================
#  visualisation_godunov.gnu  (version robuste)
#  Script de visualisation pour le schéma Godunov HLL (Saint-Venant 1D)
#  Détecte automatiquement le nombre de blocs "# time=" dans le fichier
# ============================================================

reset
set term qt size 900,500 enhanced font "Arial,10"
set datafile commentschars "#"
set xlabel "x (m)"
set grid
set key outside
set style line 1 lw 2

# --- Fichier de données ---
datafile = "solution_godunov.dat"

# --- Option de secours si grep/awk non dispo (mettre 0 pour auto) ---
# Exemple: FORCE_NBLOCKS = 5 forcera l'utilisation de 5 blocs (index 0..4)
FORCE_NBLOCKS = 0

# --- Tenter de détecter le nombre de blocs en recherchant "# time=" dans le fichier ---
# Note : system() retourne une chaîne ; on convertit ensuite en entier.
blocks_str = (FORCE_NBLOCKS > 0) ? sprintf("%d", int(FORCE_NBLOCKS)) : system(sprintf("grep -c '^# time=' %s 2>/dev/null || true", datafile))
# Fallback au cas où system() renvoie vide (Windows sans grep) -> on essaye awk
if (strlen(trim(blocks_str)) == 0) {
    blocks_str = system(sprintf("awk '/^# time=/{c++} END{print c+0}' %s 2>/dev/null || echo 0", datafile))
}

Nblocks = int(real(blocks_str))
if (Nblocks <= 0) {
    print sprintf("Erreur : aucun bloc '# time=' trouvé dans '%s'. Vérifie le fichier.", datafile)
    print "Si tu n'as pas grep/awk, définis FORCE_NBLOCKS à un entier positif en tête du script."
    exit
}

print sprintf("Nombre de blocs détectés dans '%s' : %d", datafile, Nblocks)

# On prendra pour tracés initiaux au maximum 11 courbes (0..10) ou moins si pas assez de blocs
maxPlotIndex = (Nblocks-1 < 10) ? (Nblocks-1) : 10

# ============================================================
# 1️⃣ Tracé de la hauteur h(x) à plusieurs instants
# ============================================================
set title "Hauteur d'eau h(x,t) - Schéma Godunov (HLL)"
set ylabel "Hauteur h (m)"
unset y2tics
unset y2label
set ytics nomirror
plot for [i=0:maxPlotIndex] datafile index i using 1:2 with lines lw 2 title sprintf("bloc %d", i)

pause -1 "Appuyez sur Entrée pour continuer..."

# ============================================================
# 2️⃣ Tracé de la vitesse u(x) à plusieurs instants
# ============================================================
set title "Vitesse moyenne u(x,t) - Schéma Godunov (HLL)"
set ylabel "Vitesse u (m/s)"
plot for [i=0:maxPlotIndex] datafile index i using 1:3 with lines lw 2 title sprintf("bloc %d", i)

pause -1 "Appuyez sur Entrée pour continuer..."

# ============================================================
# 3️⃣ Superposition hauteur et vitesse (axes doubles)
#    On choisit le dernier bloc disponible pour la comparaison
# ============================================================
lastIndex = Nblocks - 1
set title sprintf("Hauteur et vitesse au dernier snapshot (bloc %d)", lastIndex)
set ylabel "Hauteur h (m)"
set y2label "Vitesse u (m/s)"
set y2tics
set ytics nomirror
set key inside top right

plot datafile index lastIndex using 1:2 with lines lw 2 title "Hauteur h (x)", \
     ''            index lastIndex using 1:3 with lines lw 2 title "Vitesse u (x)" axis x1y2

pause -1 "Appuyez sur Entrée pour continuer..."

# ============================================================
# 4️⃣ Création d'une animation GIF (optionnel)
# ============================================================
set term gif animate delay 10 size 900,500
set output "animation_godunov.gif"
set title "Animation - Hauteur d'eau (Godunov HLL)"
unset y2tics
unset y2label
set ylabel "Hauteur h (m)"
set key inside top right

do for [i=0:(Nblocks-1)] {
    plot datafile index i using 1:2 with lines lw 2 title sprintf("timestep %d", i)
}
set output
print "✅ Animation sauvegardée sous 'animation_godunov.gif'"

pause -1 "Appuyez sur Entrée pour quitter..."
