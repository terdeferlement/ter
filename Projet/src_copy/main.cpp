#include "SaintVenant.h"
#include <iostream>
#include <string>

using namespace std;

int main()
{
    int choix_cas, choix_flux;
    
    cout << "========================================" << endl;
    cout << "   Résolution Saint-Venant 1D" << endl;
    cout << "========================================" << endl;
    cout << endl;
    
    // Choix du cas test
    cout << "Choisir un cas test :" << endl;
    cout << "1. Barrage de rupture (Dam Break)" << endl;
    cout << "2. Onde sinusoïdale" << endl;
    cout << "3. État uniforme" << endl;
    cout << "Votre choix (1-3) : ";
    cin >> choix_cas;
    cout << endl;
    
    // Choix du flux numérique
    cout << "Choisir un flux numérique :" << endl;
    cout << "1. Lax-Friedrichs" << endl;
    cout << "2. Rusanov" << endl;
    cout << "Votre choix (1-2) : ";
    cin >> choix_flux;
    cout << endl;
    
    // Paramètres de simulation
    int N = 200;              // Nombre de cellules
    double L = 1.0;          // Longueur du domaine
    double CFL = 0.45;       // Nombre CFL
    double t_final = 0.2;    // Temps final
    string results = "solution.txt";
    
    cout << "========================================" << endl;
    cout << "Paramètres de simulation :" << endl;
    cout << "  N = " << N << " cellules" << endl;
    cout << "  L = " << L << " m" << endl;
    cout << "  CFL = " << CFL << endl;
    cout << "  t_final = " << t_final << " s" << endl;
    cout << "========================================" << endl;
    cout << endl;
    
    // Création du flux numérique
    NumericalFlux* flux = nullptr;
    switch(choix_flux) {
        case 1:
            flux = new LaxFriedrichsFlux();
            cout << "Flux numérique : Lax-Friedrichs" << endl;
            break;
        case 2:
            flux = new RusanovFlux();
            cout << "Flux numérique : Rusanov" << endl;
            break;
        default:
            flux = new LaxFriedrichsFlux();
            cout << "Flux par défaut : Lax-Friedrichs" << endl;
    }
    
    // Création du solveur
    GodunovScheme solver;
    solver.Initialize(N, L, CFL, results, flux);
    
    // Définition de la condition initiale
    vector<State> W0;
    switch(choix_cas) {
        case 1:
            W0 = InitialCondition::DamBreak(N, 0.5);
            cout << "Cas test : Barrage de rupture (x_dam = 0.5)" << endl;
            break;
        case 2:
            W0 = InitialCondition::SineWave(N, 0.1);
            cout << "Cas test : Onde sinusoïdale (amplitude = 0.1)" << endl;
            break;
        case 3:
            W0 = InitialCondition::Uniform(N, 1.0, 0.5);
            cout << "Cas test : État uniforme (h=1.0, u=0.5)" << endl;
            break;
        default:
            W0 = InitialCondition::DamBreak(N, 0.5);
            cout << "Cas par défaut : Barrage de rupture" << endl;
    }
    
    solver.SetInitialCondition(W0);
    cout << "========================================" << endl;
    cout << endl;
    
    // Sauvegarde condition initiale
    solver.SaveSolution();
    
    // Boucle en temps
    double t = 0.0;
    int iter = 0;
    
    cout << "Début de la simulation..." << endl;
    cout << endl;
    
    while (t < t_final) {
        solver.Advance();
        t = solver.GetTime();
        iter++;
        
        if (iter % 50 == 0) {
            cout << "Itération " << iter << " : t = " << t 
                 << ", dt = " << solver.GetDt() << endl;
            solver.SaveSolution();
        }
    }
    
    // Sauvegarde finale
    solver.SaveSolution();
    
    cout << endl;
    cout << "========================================" << endl;
    cout << "Simulation terminée !" << endl;
    cout << "  Nombre d'itérations : " << iter << endl;
    cout << "  Temps final : " << t << " s" << endl;
    cout << "  Résultats dans : " << results << endl;
    cout << "========================================" << endl;
    
    // Libération mémoire
    delete flux;
    
    // Instructions pour visualiser
    cout << endl;
    cout << "Pour visualiser les résultats avec gnuplot :" << endl;
    cout << "  gnuplot> plot 'solution.txt' using 2:3 with lines" << endl;
    cout << "  gnuplot> splot 'solution.txt' using 1:2:3 with lines" << endl;
    cout << endl;
    
    return 0;
}