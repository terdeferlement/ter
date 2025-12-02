#include "SaintVenant.h"
#include <cmath>
#include <iostream>

using namespace std;

// ========================================
// Constructeur : initialise juste le temps à 0
// ========================================
SaintVenant1D::SaintVenant1D() : _t(0.0)
{
}

// ========================================
// Destructeur : ferme le fichier s'il est ouvert
// ========================================
SaintVenant1D::~SaintVenant1D()
{
    if (_fichier.is_open())
        _fichier.close();
}

// ========================================
// Initialiser la simulation
// N = nombre de cellules
// L = longueur du domaine
// CFL = nombre CFL (0.45 conseillé)
// nom_fichier = où sauvegarder les résultats
// ========================================
void SaintVenant1D::Initialiser(int N, double L, double CFL, string nom_fichier)
{
    _N = N;
    _L = L;
    _dx = L / N;  // Taille d'une cellule
    _CFL = CFL;
    _t = 0.0;
    
    // Allouer les vecteurs de taille N
    _h.resize(N);
    _hu.resize(N);
    
    // Ouvrir le fichier
    _fichier.open(nom_fichier);
    
    cout << "Simulation initialisée :" << endl;
    cout << "  - Nombre de cellules : " << N << endl;
    cout << "  - Longueur domaine : " << L << " m" << endl;
    cout << "  - Pas d'espace dx : " << _dx << " m" << endl;
}

void SaintVenant1D::ConditionInitialeDamBreak() {
    for (int i = 0; i < _N; i++) {
        if (i < _N / 2) _h[i] = 10.0; // Gauche haute
        else            _h[i] = 5.0; // Droite basse
        _hu[i] = 0.0;
    }
}

// ========================================
// Condition initiale : Houle (Soliton)
// Une vague unique qui se déplace vers la droite
// ========================================
void SaintVenant1D::ConditionInitialeHoule(double amplitude)
{
    double h_fond = 1;            // Profondeur au repos
    double x_centre = 0.5 * _L;     // On la fait partir de la gauche (30% du domaine)
    
    // Calcul de la largeur de la vague (relation physique du soliton)
    // k influence la "finesse" de la vague
    double k = sqrt((3.0 * amplitude) / (4.0 * pow(h_fond, 3)));
    
    // Célérité de l'onde (vitesse de phase approx)
    double c = sqrt(_g * (h_fond + amplitude));

    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        // 1. Forme de la surface : Profil en sech²
        double arg = k * (x - x_centre);
        double sech = 1.0 / cosh(arg);
        double perturbation = amplitude * (sech * sech);
        
        _h[i] = h_fond + perturbation;
        
        // 2. Vitesse initiale : CRUCIAL
        // Si on met 0, la vague se coupe en deux.
        // On initialise u pour que la vague aille vers la DROITE.
        // Relation approchée : u = c * (h - h_fond) / h
        
        
        if (_h[i] > 1e-6) {
             _hu[i] = c * ((_h[i] - h_fond) / _h[i]) * _h[i]; // donc c * perturbation
        } else {
             _hu[i] = 0.0;
        }
    }
    
    cout << "Condition initiale : Houle (Soliton) vers la droite, Amp=" << amplitude << endl;
}


// ========================================
// Calculer le flux physique F(W)
// Pour Saint-Venant : F = (hu, hu²/h + g*h²/2)
// ========================================
void SaintVenant1D::CalculerFluxPhysique(double h, double hu, double& F_h, double& F_hu)
{
    // Première composante : F_h = hu
    F_h = hu;
    
    // Deuxième composante : F_hu = hu²/h + g*h²/2
    if (h > critere_hauteur_deau)  // Éviter division par zéro
    {
        double u = hu / h;
        F_hu = hu * u + 0.5 * _g * h * h;
    }
    else
    {
        F_hu = 0.0;
    }
}

// ========================================
// Calculer la vitesse u = hu/h
// ========================================
double SaintVenant1D::CalculerVitesse(double h, double hu)
{
    if (h > critere_hauteur_deau)
        return hu / h;
    else
        return 0.0;
}

// ========================================
// Flux numérique de Lax-Friedrichs
// C'est la moyenne des flux + un terme de dissipation
// ========================================
void SaintVenant1D::FluxRusanov(double hL, double huL, double hR, double huR,
                                       double& flux_h, double& flux_hu)
{
    // 1. Calculer les flux physiques à gauche et à droite
    double FL_h, FL_hu;   // Flux gauche
    double FR_h, FR_hu;   // Flux droite
    
    CalculerFluxPhysique(hL, huL, FL_h, FL_hu);
    CalculerFluxPhysique(hR, huR, FR_h, FR_hu);
    
    // 2. Calculer les vitesses de propagation
    double uL = CalculerVitesse(hL, huL);
    double uR = CalculerVitesse(hR, huR);
    
    double cL = 0.0, cR = 0.0;
    if (hL > critere_hauteur_deau) cL = sqrt(_g * hL);  // Célérité gauche
    if (hR > critere_hauteur_deau) cR = sqrt(_g * hR);  // Célérité droite
    
    double lambdaL = abs(uL) + cL;  // Vitesse max à gauche
    double lambdaR = abs(uR) + cR;  // Vitesse max à droite
    double lambda = max(lambdaL, lambdaR);  // On prend le maximum
    
    // 3. Flux de Rusanov = moyenne + dissipation
    flux_h = 0.5 * (FL_h + FR_h) - 0.5 * lambda * (hR - hL);
    flux_hu = 0.5 * (FL_hu + FR_hu) - 0.5 * lambda * (huR - huL);
}

// ========================================
// Calculer la vitesse maximale dans le domaine
// Sert pour la condition CFL
// ========================================
double SaintVenant1D::VitesseMaximale()
{
    double v_max = 0.0;
    
    for (int i = 0; i < _N; i++)
    {
        double u = CalculerVitesse(_h[i], _hu[i]);
        double c = 0.0;
        if (_h[i] > 1e-10) 
            c = sqrt(_g * _h[i]);
        
        double vitesse = fabs(u) + c;
        v_max = max(v_max, vitesse);
    }
    
    return v_max;
}

// ========================================
// Calculer le pas de temps avec CFL
// dt = CFL * dx / vitesse_max
// ========================================
void SaintVenant1D::CalculerPasDeTemps()
{
    double v_max = VitesseMaximale();
    
    if (v_max > critere_vitesse)
        _dt = _CFL * _dx / v_max;
    else
        _dt = 0.01;  // Valeur par défaut si v_max = 0
}

// ========================================
// Avancer d'un pas de temps
// Schéma de Godunov avec flux de Lax-Friedrichs
// ========================================
void SaintVenant1D::Avancer()
{
    // 1. Calculer le nouveau pas de temps
    CalculerPasDeTemps();
    
    // 2. Créer des vecteurs temporaires pour la nouvelle solution
    vector<double> h_nouveau(_N);
    vector<double> hu_nouveau(_N);
    
    // 3. Pour chaque cellule (sauf les bords)
    for (int i = 1; i < _N - 1; i++)
    {
        // Flux à l'interface droite (entre i et i+1)
        double flux_droite_h, flux_droite_hu;
        FluxRusanov(_h[i], _hu[i], _h[i+1], _hu[i+1],
                         flux_droite_h, flux_droite_hu);
        
        // Flux à l'interface gauche (entre i-1 et i)
        double flux_gauche_h, flux_gauche_hu;
        FluxRusanov(_h[i-1], _hu[i-1], _h[i], _hu[i],
                         flux_gauche_h, flux_gauche_hu);
        
        // Mise à jour conservative :
        // W_nouveau = W_ancien - (dt/dx) * (Flux_droite - Flux_gauche)
        double coeff = _dt / _dx;
        h_nouveau[i] = _h[i] - coeff * (flux_droite_h - flux_gauche_h);
        //hu_nouveau[0] = hu_nouveau[1]; // On copie la vitesse
        hu_nouveau[i] = _hu[i] - coeff * (flux_droite_hu - flux_gauche_hu);
    }
    
    // 4. Conditions aux limites : réflexion
    h_nouveau[0] = h_nouveau[1];
    hu_nouveau[0] = hu_nouveau[1];
    h_nouveau[_N-1] = h_nouveau[_N-2];
    hu_nouveau[_N-1] = hu_nouveau[_N-2];


    
    // 5. Copier la nouvelle solution
    _h = h_nouveau;
    _hu = hu_nouveau;
    
    // 6. Avancer le temps
    _t += _dt;
}

// ========================================
// Sauvegarder la solution dans le fichier
// Format : temps x h u
// ========================================
void SaintVenant1D::Sauvegarder()
{
    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;  // Centre de la cellule
        double u = CalculerVitesse(_h[i], _hu[i]);
        
        _fichier << _t << " " << x << " " << _h[i] << " " << u << endl;
    }
    _fichier << endl;  // Ligne vide pour séparer les temps
}




double SaintVenant1D::CalculerMasseTotale()
{
    double volume_total = 0.0;
    
    // On somme la hauteur d'eau de toutes les cellules
    for (int i = 0; i < _N; i++)
    {
        volume_total += _h[i];
    }
    
    // Volume = Somme des hauteurs * largeur d'une cellule
    return volume_total * _dx;
}