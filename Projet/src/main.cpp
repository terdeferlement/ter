#include "SaintVenant.h"
#include <iostream>
#include <cmath>


using namespace std;

int main()
{
    cout << "========================================" << endl;
    cout << "   Saint-Venant 1D - Version Simple" << endl;
    cout << "========================================" << endl;
    cout << endl;
    
    // ========================================
    // PARAMÈTRES DE LA SIMULATION
    // ========================================
    int N = 6000;              // Nombre de cellules dans le domaine
    double L = 90.0;           // Longueur du domaine (en mètres)
    double CFL = 0.45;        // Nombre CFL (doit être < 0.5)
    double t_final = 15;     // Temps final de simulation (secondes)
    string fichier = "solution.txt";  // Fichier de sortie
    
    cout << "Paramètres :" << endl;
    cout << "  Nombre de cellules : " << N << endl;
    cout << "  Longueur domaine : " << L << " m" << endl;
    cout << "  Temps final : " << t_final << " s" << endl;
    cout << "  CFL : " << CFL << endl;
    cout << endl;
    
    // ========================================
    // CRÉATION DU SOLVEUR
    // ========================================
    SaintVenant1D solveur;
    
    // Initialiser avec les paramètres
    solveur.Initialiser(N, L, CFL, fichier);
    cout << endl;
    
// ========================================
    // CONDITION INITIALE : Houle
    // ========================================
    // Amplitude 0.3m sur 1m de fond = belle vague bien visible
    double amplitude_vague(0.2);
    solveur.ConditionInitialeHoule(amplitude_vague);

    // solveur.ConditionInitialeDamBreak();





    // On stocke la position de départ de la vague
    double x_depart = solveur.ObtenirPositionCrete();

    double masse_initiale = solveur.CalculerMasseTotale();
    cout << "Masse initiale du système : " << masse_initiale << " m^2" << endl;

    double h_fond = solveur.ObtenirHFond();  // La profondeur mise dans SaintVenant.cpp
    double amplitude = amplitude_vague; // L'amplitude  choisie

    // 1. Hauteur totale au sommet de la vague
    double h_crete = h_fond + amplitude;
    // 2. Célérité locale au sommet (c = sqrt(g*h))
    double c_crete = sqrt(9.81 * h_crete);
    // 3. Vitesse du fluide au sommet (u)
    // D'après l'initialisation : u = c_soliton * (Amplitude / h_crete)
    double u_crete = c_crete * (amplitude / h_crete);
    // 4. Vitesse théorique Saint-Venant (V = u + c)
    double vitesse_theorique = u_crete + c_crete;
    
    cout << "Position initiale de la crete : " << x_depart << " m" << endl;
    cout << "Vitesse Theorique attendue   : " << vitesse_theorique << " m/s" << endl;
    double energie_initiale = solveur.CalculerEnergieTotale();
    cout << "Energie initiale : " << energie_initiale << " J" << endl;
    cout << endl;
    // ------------------------------------------------




    // Sauvegarder l'état initial
    solveur.Sauvegarder();
    cout << endl;
    

    // ========================================
    // BOUCLE EN TEMPS
    // ========================================
    cout << "Début de la simulation..." << endl;
    cout << endl;
    
    int iteration = 0;
    double t = 0.0;
    
    while (t < t_final)
    {
        // Avancer d'un pas de temps
        solveur.Avancer();
        
        // Récupérer le temps actuel
        t = solveur.ObtenirTemps();
        iteration++;
        
        // Afficher l'avancement tous les 50 pas
        if (iteration % 50 == 0)
        {

        // ========================================
        //Validation de la masse
        // ========================================
        // 1. Calculer la masse actuelle et l'erreur
        double masse_actuelle = solveur.CalculerMasseTotale();
        double erreur_masse = masse_actuelle - masse_initiale;
        
        cout << "Itération " << iteration 
                << " : t = " << t 
                << " s, dt = " << solveur.ObtenirDt() << " s" << endl;
                
        // 2. Affichage scientifique pour la petite erreur
        cout << "  -> Erreur de masse (M_actu - M_init) : " 
                << scientific << erreur_masse 
                << " (doit etre tres proche de 0)" << endl;
        cout.unsetf(ios::scientific); // Revenir à l'affichage normal
    
        // ========================================
        //Validation de la vitesse
        // ========================================

        // 1. Où est la vague maintenant ?
        double x_actuel = solveur.ObtenirPositionCrete();
        
        // 2. Quelle distance a-t-elle parcourue ?
        double distance = x_actuel - x_depart;
        
        // 3. Vitesse = Distance / Temps
        // (On évite la division par zéro au tout début)
        double vitesse_mesuree = 0.0;
        if (t > 0.001) vitesse_mesuree = distance / t;
        
        // 4. On surveille aussi la hauteur (voir si elle s'écrase)
        double h_max_actuel = solveur.ObtenirHauteurMax();
        
        cout << "  -> Position Crete : " << x_actuel << " m" << endl;
        cout << "  -> Vitesse Moyenne : " << vitesse_mesuree << " m/s" 
             << " (Theo: " << vitesse_theorique << ")" << endl;
        cout << "  -> Hauteur Max    : " << h_max_actuel << " m" << endl;

        // ========================================
        //Validation de l'energie 
        // ========================================

        // 1. Calculs
        double energie_actuelle = solveur.CalculerEnergieTotale();
        double erreur_energie = energie_actuelle - energie_initiale; // L'énergie diminue un peu, c'est normal (dissipation)

        // 2. Affichage complet
        cout << "  -> Err Masse   : " << scientific << erreur_masse << endl;
        cout << "  -> Err Energie : " << scientific << erreur_energie << " (Doit diminuer legerement)" << endl; 
        cout.unsetf(ios::scientific);


        cout << endl;
        // ------------------------------------------------


        // Sauvegarder de temps en temps
        solveur.Sauvegarder();

        }
    }
    
    // Sauvegarder l'état final
    solveur.Sauvegarder();
    
    cout << endl;
    cout << "========================================" << endl;
    cout << "Simulation terminée !" << endl;
    cout << "  Nombre d'itérations : " << iteration << endl;
    cout << "  Temps final : " << t << " s" << endl;
    cout << "  Résultats dans : " << fichier << endl;
    cout << "========================================" << endl;
    cout << endl;
    
    // ========================================
    // INSTRUCTIONS POUR VISUALISER
    // ========================================
    cout << "Pour visualiser les résultats :" << endl;
    cout << endl;
    cout << "Avec Python :" << endl;
    cout << "  python3 visualize.py" << endl;
    cout << endl;
    cout << "Avec gnuplot :" << endl;
    cout << "  gnuplot" << endl;
    cout << "  gnuplot> plot 'solution.txt' using 2:3 with lines" << endl;
    cout << endl;



    
    
    return 0;
}