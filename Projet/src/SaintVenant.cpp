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
    _zb.resize(N); 
    
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
// Condition initiale : Houle 
// Une vague unique qui se déplace vers la droite
// ========================================

void SaintVenant1D::ConditionInitialeHoule(double amplitude)
{
    double niveau_eau_moyen = 1.0;  // Niveau de la mer (Z = 1m)
    _h_fond = niveau_eau_moyen;     // Pour le calcul théorique
    
    // Paramètres de la vague (Soliton)
    double x_vague = 0.5*_L; // Départ de la vague (zone plate)
    double k = sqrt((3.0 * amplitude) / (4.0 * pow(niveau_eau_moyen, 3)));
    double c = sqrt(_g * (niveau_eau_moyen + amplitude));

    // Paramètres de la Plage
    double x_debut_pente = _L / 2.0; // Commence au milieu (25m)
    double z_fin = 0;              // Monte jusqu'à 2m de haut à la fin
    double pente = (z_fin - 0.0) / (_L - x_debut_pente); 

    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        // 1. Calcul du FOND (Bathymétrie)
        if (x < x_debut_pente) {
            _zb[i] = 0.0; // Fond plat
        } else {
            _zb[i] = pente * (x - x_debut_pente); // Montée linéaire
        }
        
        // 2. Calcul de la SURFACE (Vague + Niveau moyen)
        double arg = k * (x - x_vague);
        double sech = 1.0 / cosh(arg);
        double perturbation = amplitude * (sech * sech);
        
        double surface_eau = niveau_eau_moyen + perturbation;
        
        // 3. Calcul de la HAUTEUR D'EAU (h = Surface - Fond)
        _h[i] = surface_eau - _zb[i];
        
        // Gestion de la zone sèche (si la plage dépasse l'eau)
        if (_h[i] < 0) _h[i] = 0.0;

        // 4. Vitesse
        // On ne met de la vitesse que là où il y a de l'eau
        if (_h[i] > 1e-6) {
             // Approximation : la vitesse suit la perturbation de surface
             // Attention : sur la pente, cette init est approximative mais ok
             double h_theorique_plat = niveau_eau_moyen + perturbation;
             double u_theorique = c * ((h_theorique_plat - niveau_eau_moyen) / h_theorique_plat);
             
             _hu[i] = _h[i] * u_theorique;
        } else {
             _hu[i] = 0.0;
        }
    }
    cout << "Plage initialisée : Pente de " << x_debut_pente << "m a " << _L << "m." << endl;
}



void SaintVenant1D::ConditionInitialeGaussienne(double amplitude)
{
    double niveau_surface_libre = 1.0; // Altitude de la surface libre (Z absolu)
    _h_fond = niveau_surface_libre; // Pour référence
    
    double x_centre = 0.25*_L;
    double largeur = 0.5;

    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        // 1. Surface libre de base (horizontale)
        double surface_base = niveau_surface_libre;
        
        // 2. Ajouter la perturbation gaussienne sur la SURFACE
        double distance = x - x_centre;
        double perturbation = amplitude * exp(-(distance*distance)/(2.0*largeur*largeur));
        double surface_totale = surface_base + perturbation;
        
        // 3. Calculer h = Surface - Fond
        _h[i] = surface_totale - _zb[i];
        
        // Gestion zone sèche
        if (_h[i] < 0) _h[i] = 0.0;
        
        // 4. Vitesse initiale nulle
        _hu[i] = 0.0;
    }
    cout << "Fluide : Gaussienne (Amp=" << amplitude 
         << ", Surface=" << niveau_surface_libre << "m) sur bathymétrie." << endl;
}
// Condition initiale de bathymetrie

void SaintVenant1D::DefinirFondPlat()
{
    // Remplir tout avec 0
    for (int i = 0; i < _N; i++)
    {
        _zb[i] = 0.0;
    }
    _h_fond = 1.0; // Valeur par défaut pour référence
    cout << "Bathymetrie : Fond plat (z=0)." << endl;
}

void SaintVenant1D::DefinirFondPente(double x_debut, double z_fin)
{
    // Pente linéaire à partir de x_debut
    double pente = z_fin / (_L - x_debut);
    
    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        if (x < x_debut) _zb[i] = 0.0;
        else             _zb[i] = pente * (x - x_debut);
    }
    cout << "Bathymetrie : Pente démarrant a x=" << x_debut << "m." << endl;
}

void SaintVenant1D::DefinirFondMarche(double x_marche, double z_haut)
{
    for (int i = 0; i < _N; i++)
    {
        double x = (i + 0.5) * _dx;
        
        if (x < x_marche)
        {
            _zb[i] = 0.0;     // Partie basse (avant la marche)
        }
        else
        {
            _zb[i] = z_haut;  // Partie haute (sur la marche)
        }
    }
    cout << "Bathymetrie : Marche d'escalier a x=" << x_marche << "m (Hauteur=" << z_haut << "m)." << endl;
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
        hu_nouveau[i] = _hu[i] - coeff * (flux_droite_hu - flux_gauche_hu);

        

  // ===== TERME SOURCE WELL-BALANCED =====
        // Pour préserver l'équilibre hydrostatique (lac au repos)
        // On doit calculer le gradient de la SURFACE LIBRE et non du fond
        if (_h[i] > critere_hauteur_deau)
        {
            // Surface libre aux interfaces
            double surface_gauche = _h[i-1] + _zb[i-1];
            double surface_droite = _h[i+1] + _zb[i+1];
            
            // Gradient de surface (pente de l'eau)
            double grad_surface = (surface_droite - surface_gauche) / (2.0 * _dx);
            
            // Terme source : force seulement si la surface n'est PAS horizontale
            double Source = - _g * _h[i] * grad_surface;
            hu_nouveau[i] += _dt * Source;
        }

    }
    
    //Conditions limite fenetre ouverte
    h_nouveau[0] = h_nouveau[1];
    hu_nouveau[0] = hu_nouveau[1];
    h_nouveau[_N-1] = h_nouveau[_N-2];
    hu_nouveau[_N-1] = hu_nouveau[_N-2];

    // // 4. Conditions aux limites : réflexion
    // h_nouveau[0] = h_nouveau[1];
    // hu_nouveau[0] = -hu_nouveau[1];
    // h_nouveau[_N-1] = h_nouveau[_N-2];
    // hu_nouveau[_N-1] = -hu_nouveau[_N-2];


    
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
        double x = (i + 0.5) * _dx;
        double u = CalculerVitesse(_h[i], _hu[i]);
        double zb = _zb[i];
        double H = _h[i] + zb; // Surface libre (Niveau de l'eau)
        
        // On écrit : t x h u zb H
        _fichier << _t << " " << x << " " << _h[i] << " " << u << " " << zb << " " << H << endl;
    }
    _fichier << endl;
}



// Validation


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


double SaintVenant1D::ObtenirHauteurMax()
{
    double h_max = -1.0;
    for (int i = 0; i < _N; i++)
    {
        if (_h[i] > h_max) h_max = _h[i];
    }
    return h_max;
}

double SaintVenant1D::ObtenirPositionCrete()
{
    double h_max = -1.0;
    int i_max = 0;
    
    // Trouver l'indice de la cellule où l'eau est la plus haute
    for (int i = 0; i < _N; i++)
    {
        if (_h[i] > h_max)
        {
            h_max = _h[i];
            i_max = i;
        }
    }
    
    // Retourner la position physique (x) de cette cellule
    return (i_max + 0.5) * _dx;
}

double SaintVenant1D::CalculerEnergieTotale()
{
    double energie_totale = 0.0;
    
    for (int i = 0; i < _N; i++)
    {
        // 1. Energie Potentielle : 1/2 * g * h^2
        double Ep = 0.5 * _g * _h[i] * _h[i];
        
        // 2. Energie Cinétique : 1/2 * h * u^2  (ou 1/2 * (hu)^2 / h)
        double Ec = 0.0;
        if (_h[i] > 1e-10) {
            double u = _hu[i] / _h[i];
            Ec = 0.5 * _h[i] * u * u;
        }
        
        energie_totale += (Ep + Ec);
    }
    
    return energie_totale * _dx;
}