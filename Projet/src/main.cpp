#include "SaintVenant.h"
#include <iostream>

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
    int N = 200;              // Nombre de cellules dans le domaine
    double L = 1.0;           // Longueur du domaine (en mètres)
    double CFL = 0.45;        // Nombre CFL (doit être < 0.5)
    double t_final = 0.2;     // Temps final de simulation (secondes)
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
    solveur.ConditionInitialeHoule(0.3);
    
    
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
            cout << "Itération " << iteration 
                 << " : t = " << t 
                 << " s, dt = " << solveur.ObtenirDt() << " s" << endl;
            
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