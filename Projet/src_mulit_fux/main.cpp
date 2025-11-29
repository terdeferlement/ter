#include "SaintVenant.h"
#include <iostream>

using namespace std;

int main()
{
    cout << "========================================" << endl;
    cout << "   Saint-Venant 1D - Version Multi-Flux" << endl;
    cout << "========================================" << endl;
    cout << endl;
    
    // ========================================
    // PARAMÈTRES DE LA SIMULATION
    // ========================================
    int N = 200;              // Nombre de cellules dans le domaine
    double L = 1.0;           // Longueur du domaine (en mètres)
    double CFL = 0.45;        // Nombre CFL (doit être < 0.5)
    double t_final = 0.2;     // Temps final de simulation (secondes)
    
    cout << "Paramètres :" << endl;
    cout << "  Nombre de cellules : " << N << endl;
    cout << "  Longueur domaine : " << L << " m" << endl;
    cout << "  Temps final : " << t_final << " s" << endl;
    cout << "  CFL : " << CFL << endl;
    cout << endl;
    
    // ========================================
    // CHOIX DU FLUX NUMÉRIQUE
    // ========================================
    // Décommentez la ligne correspondant au flux que vous voulez tester
    
    //TypeFlux flux = FLUX_LAX_FRIEDRICHS;  // Flux de Lax-Friedrichs
    //TypeFlux flux = FLUX_RUSANOV;          // Flux de Rusanov (identique à LF)
    TypeFlux flux = FLUX_HLL;              // Flux HLL (plus précis)
    
    // ========================================
    // CAS DE TEST 1 : Lax-Friedrichs (alpha GLOBAL)
    // ========================================
    cout << "--- Test avec Lax-Friedrichs (alpha global) ---" << endl;
    {
        SaintVenant1D solveur;
        solveur.Initialiser(N, L, CFL, "solution_LF.txt", FLUX_LAX_FRIEDRICHS);
        solveur.ConditionInitialeBarrage(0.5);
        solveur.Sauvegarder();
        
        int iteration = 0;
        double t = 0.0;
        while (t < t_final)
        {
            solveur.Avancer();
            t = solveur.ObtenirTemps();
            iteration++;
            
            if (iteration % 50 == 0)
            {
                cout << "Itération " << iteration << " : t = " << t << " s" << endl;
                solveur.Sauvegarder();
            }
        }
        solveur.Sauvegarder();
        cout << "Terminé : " << iteration << " itérations" << endl;
        cout << endl;
    }
    
    // ========================================
    // CAS DE TEST 2 : Rusanov (alpha LOCAL)
    // ========================================
    cout << "--- Test avec Rusanov (alpha local) ---" << endl;
    {
        SaintVenant1D solveur;
        solveur.Initialiser(N, L, CFL, "solution_Rusanov.txt", FLUX_RUSANOV);
        solveur.ConditionInitialeBarrage(0.5);
        solveur.Sauvegarder();
        
        int iteration = 0;
        double t = 0.0;
        while (t < t_final)
        {
            solveur.Avancer();
            t = solveur.ObtenirTemps();
            iteration++;
            
            if (iteration % 50 == 0)
            {
                cout << "Itération " << iteration << " : t = " << t << " s" << endl;
                solveur.Sauvegarder();
            }
        }
        solveur.Sauvegarder();
        cout << "Terminé : " << iteration << " itérations" << endl;
        cout << endl;
    }
    
    // ========================================
    // CAS DE TEST 3 : HLL
    // ========================================
    cout << "--- Test avec HLL ---" << endl;
    {
        SaintVenant1D solveur;
        solveur.Initialiser(N, L, CFL, "solution_HLL.txt", FLUX_HLL);
        solveur.ConditionInitialeBarrage(0.5);
        solveur.Sauvegarder();
        
        int iteration = 0;
        double t = 0.0;
        while (t < t_final)
        {
            solveur.Avancer();
            t = solveur.ObtenirTemps();
            iteration++;
            
            if (iteration % 50 == 0)
            {
                cout << "Itération " << iteration << " : t = " << t << " s" << endl;
                solveur.Sauvegarder();
            }
        }
        solveur.Sauvegarder();
        cout << "Terminé : " << iteration << " itérations" << endl;
        cout << endl;
    }
    
    // ========================================
    // CAS DE TEST 4 : Roe
    // ========================================
    cout << "--- Test avec Roe ---" << endl;
    {
        SaintVenant1D solveur;
        solveur.Initialiser(N, L, CFL, "solution_Roe.txt", FLUX_ROE);
        solveur.ConditionInitialeBarrage(0.5);
        solveur.Sauvegarder();
        
        int iteration = 0;
        double t = 0.0;
        while (t < t_final)
        {
            solveur.Avancer();
            t = solveur.ObtenirTemps();
            iteration++;
            
            if (iteration % 50 == 0)
            {
                cout << "Itération " << iteration << " : t = " << t << " s" << endl;
                solveur.Sauvegarder();
            }
        }
        solveur.Sauvegarder();
        cout << "Terminé : " << iteration << " itérations" << endl;
        cout << endl;
    }
    
    cout << "========================================" << endl;
    cout << "Toutes les simulations terminées !" << endl;
    cout << "  Résultats dans :" << endl;
    cout << "    - solution_LF.txt (Lax-Friedrichs - alpha global)" << endl;
    cout << "    - solution_Rusanov.txt (Rusanov - alpha local)" << endl;
    cout << "    - solution_HLL.txt (HLL)" << endl;
    cout << "    - solution_Roe.txt (Roe)" << endl;
    cout << "========================================" << endl;
    cout << endl;
    
    // ========================================
    // INSTRUCTIONS POUR VISUALISER
    // ========================================
    cout << "Pour comparer les résultats avec gnuplot :" << endl;
    cout << "  gnuplot" << endl;
    cout << "  gnuplot> plot 'solution_LF.txt' u 2:3 w l title 'LF (global)', \\" << endl;
    cout << "               'solution_Rusanov.txt' u 2:3 w l title 'Rusanov (local)', \\" << endl;
    cout << "               'solution_HLL.txt' u 2:3 w l title 'HLL', \\" << endl;
    cout << "               'solution_Roe.txt' u 2:3 w l title 'Roe'" << endl;
    cout << endl;
    
    return 0;
}