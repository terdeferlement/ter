#include "SaintVenant.h"
#include <cmath>
#include <iostream>
#include <algorithm>

using namespace std;

// ========================================
// Constructeur : initialise juste le temps à 0
// ========================================
SaintVenant1D::SaintVenant1D() : _t(0.0), _type_flux(FLUX_LAX_FRIEDRICHS)
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
// type_flux = type de flux numérique à utiliser
// ========================================
void SaintVenant1D::Initialiser(int N, double L, double CFL, string nom_fichier, TypeFlux type_flux)
{
    _N = N;
    _L = L;
    _dx = L / N;  // Taille d'une cellule
    _CFL = CFL;
    _t = 0.0;
    _type_flux = type_flux;
    
    // Allouer les vecteurs de taille N
    _h.resize(N);
    _hu.resize(N);
    
    // Ouvrir le fichier
    _fichier.open(nom_fichier);
    
    cout << "Simulation initialisée :" << endl;
    cout << "  - Nombre de cellules : " << N << endl;
    cout << "  - Longueur domaine : " << L << " m" << endl;
    cout << "  - Pas d'espace dx : " << _dx << " m" << endl;
    cout << "  - Flux numérique : " << ObtenirNomFlux() << endl;
}

// ========================================
// Obtenir le nom du flux utilisé
// ========================================
string SaintVenant1D::ObtenirNomFlux() const
{
    switch (_type_flux)
    {
        case FLUX_LAX_FRIEDRICHS: return "Lax-Friedrichs";
        case FLUX_RUSANOV: return "Rusanov";
        case FLUX_HLL: return "HLL";
        case FLUX_ROE: return "Roe";
        default: return "Inconnu";
    }
}

// ========================================
// Condition initiale : barrage de rupture
// Eau haute à gauche, eau basse à droite
// ========================================
void SaintVenant1D::ConditionInitialeBarrage(double x_barrage)
{
    for (int i = 0; i < _N; i++)
    {
        // Position du centre de la cellule i
        double x = (i + 0.5) * _dx;
        
        if (x < x_barrage)
        {
            _h[i] = 2.0;   // Hauteur gauche = 2 m
            _hu[i] = 0.0;  // Vitesse nulle au départ
        }
        else
        {
            _h[i] = 0.5;   // Hauteur droite = 0.5 m
            _hu[i] = 0.0;  // Vitesse nulle au départ
        }
    }
    
    cout << "Condition initiale : barrage en x = " << x_barrage << endl;
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
    if (h > 1e-10)  // Éviter division par zéro
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
    if (h > 1e-10)
        return hu / h;
    else
        return 0.0;
}

// ========================================
// FLUX NUMÉRIQUE : appelle le bon flux selon le type choisi
// ========================================
void SaintVenant1D::CalculerFluxNumerique(double hL, double huL, double hR, double huR,
                                          double& flux_h, double& flux_hu)
{
    switch (_type_flux)
    {
        case FLUX_LAX_FRIEDRICHS:
            FluxLaxFriedrichs(hL, huL, hR, huR, flux_h, flux_hu);
            break;
            
        case FLUX_RUSANOV:
            FluxRusanov(hL, huL, hR, huR, flux_h, flux_hu);
            break;
            
        case FLUX_HLL:
            FluxHLL(hL, huL, hR, huR, flux_h, flux_hu);
            break;
            
        case FLUX_ROE:
            FluxRoe(hL, huL, hR, huR, flux_h, flux_hu);
            break;
    }
}

// ========================================
// FLUX DE LAX-FRIEDRICHS (version GLOBALE)
// Formule : F_LF = 0.5*(F_L + F_R) - 0.5*alpha*(W_R - W_L)
// où alpha = max sur TOUT LE DOMAINE de (|u|+c)
//
// IMPORTANT : Le "vrai" Lax-Friedrichs utilise alpha GLOBAL,
// c'est-à-dire le maximum sur tout le domaine spatial.
// C'est très dissipatif mais extrêmement robuste.
// ========================================
void SaintVenant1D::FluxLaxFriedrichs(double hL, double huL, double hR, double huR,
                                      double& flux_h, double& flux_hu)
{
    // 1. Calculer les flux physiques à gauche et à droite
    double FL_h, FL_hu;   // Flux gauche
    double FR_h, FR_hu;   // Flux droite
    
    CalculerFluxPhysique(hL, huL, FL_h, FL_hu);
    CalculerFluxPhysique(hR, huR, FR_h, FR_hu);
    
    // 2. Utiliser la vitesse MAXIMALE de TOUT le domaine
    // (C'est ça qui fait la différence avec Rusanov !)
    double alpha = VitesseMaximale();  // <- GLOBAL !
    
    // 3. Flux de Lax-Friedrichs = moyenne + dissipation globale
    flux_h = 0.5 * (FL_h + FR_h) - 0.5 * alpha * (hR - hL);
    flux_hu = 0.5 * (FL_hu + FR_hu) - 0.5 * alpha * (huR - huL);
}

// ========================================
// FLUX DE RUSANOV
// Formule : F_Rusanov = 0.5*(F_L + F_R) - 0.5*alpha*(W_R - W_L)
// où alpha = max(|u_L|+c_L, |u_R|+c_R) calculé LOCALEMENT
//
// DIFFÉRENCE avec Lax-Friedrichs :
// - LF : alpha = max sur TOUT le domaine (très dissipatif)
// - Rusanov : alpha = max local à cette interface (moins dissipatif)
//
// Rusanov est donc une version "locale" et améliorée de LF.
// ========================================
void SaintVenant1D::FluxRusanov(double hL, double huL, double hR, double huR,
                                double& flux_h, double& flux_hu)
{
    // 1. Calculer les flux physiques à gauche et à droite
    double FL_h, FL_hu;   // Flux gauche
    double FR_h, FR_hu;   // Flux droite
    
    CalculerFluxPhysique(hL, huL, FL_h, FL_hu);
    CalculerFluxPhysique(hR, huR, FR_h, FR_hu);
    
    // 2. Calculer les vitesses de propagation LOCALES à cette interface
    double uL = CalculerVitesse(hL, huL);
    double uR = CalculerVitesse(hR, huR);
    
    double cL = 0.0, cR = 0.0;
    if (hL > 1e-10) cL = sqrt(_g * hL);  // Célérité gauche
    if (hR > 1e-10) cR = sqrt(_g * hR);  // Célérité droite
    
    double alphaL = fabs(uL) + cL;  // Vitesse max à gauche
    double alphaR = fabs(uR) + cR;  // Vitesse max à droite
    double alpha = max(alphaL, alphaR);  // <- LOCAL à cette interface !
    
    // 3. Flux de Rusanov = moyenne + dissipation locale
    flux_h = 0.5 * (FL_h + FR_h) - 0.5 * alpha * (hR - hL);
    flux_hu = 0.5 * (FL_hu + FR_hu) - 0.5 * alpha * (huR - huL);
}

// ========================================
// FLUX DE ROE
// Le flux de Roe utilise une linéarisation exacte du système.
// Il calcule les valeurs propres et vecteurs propres de la matrice
// jacobienne "moyennée" entre gauche et droite.
//
// Pour Saint-Venant, on utilise les moyennes de Roe :
// h_avg = (h_L + h_R) / 2
// u_avg = (sqrt(h_L)*u_L + sqrt(h_R)*u_R) / (sqrt(h_L) + sqrt(h_R))
// c_avg = sqrt(g * h_avg)
//
// Les valeurs propres sont : lambda_1 = u_avg - c_avg
//                            lambda_2 = u_avg + c_avg
//
// Le flux de Roe est ensuite :
// F_Roe = 0.5*(F_L + F_R) - 0.5*|A_avg|*(W_R - W_L)
// où |A_avg| = R * |Lambda| * L avec R,L les vecteurs propres
// ========================================
void SaintVenant1D::FluxRoe(double hL, double huL, double hR, double huR,
                            double& flux_h, double& flux_hu)
{
    // 1. Calculer les flux physiques à gauche et à droite
    double FL_h, FL_hu;
    double FR_h, FR_hu;
    
    CalculerFluxPhysique(hL, huL, FL_h, FL_hu);
    CalculerFluxPhysique(hR, huR, FR_h, FR_hu);
    
    // 2. Cas particulier : si h très petit des deux côtés
    if (hL < 1e-10 && hR < 1e-10)
    {
        flux_h = 0.0;
        flux_hu = 0.0;
        return;
    }
    
    // 3. Calculer les vitesses
    double uL = CalculerVitesse(hL, huL);
    double uR = CalculerVitesse(hR, huR);
    
    // 4. Calculer les moyennes de Roe
    double sqrt_hL = sqrt(max(hL, 0.0));
    double sqrt_hR = sqrt(max(hR, 0.0));
    double sqrt_sum = sqrt_hL + sqrt_hR;
    
    // Moyenne de Roe pour h (simple moyenne arithmétique)
    double h_avg = 0.5 * (hL + hR);
    
    // Moyenne de Roe pour u (moyenne pondérée par sqrt(h))
    double u_avg;
    if (sqrt_sum > 1e-10)
        u_avg = (sqrt_hL * uL + sqrt_hR * uR) / sqrt_sum;
    else
        u_avg = 0.0;
    
    // Célérité moyenne
    double c_avg = sqrt(_g * h_avg);
    
    // 5. Calculer les valeurs propres
    double lambda1 = u_avg - c_avg;  // Onde qui va vers la gauche
    double lambda2 = u_avg + c_avg;  // Onde qui va vers la droite
    
    // 6. Calculer les sauts de variables conservatives
    double delta_h = hR - hL;
    double delta_hu = huR - huL;
    
    // 7. Calculer les coefficients alpha (projection sur les vecteurs propres)
    // Ces coefficients mesurent "combien" de chaque onde est présente
    double alpha1 = 0.5 * (delta_h - h_avg * delta_hu / c_avg) / h_avg;
    double alpha2 = 0.5 * (delta_h + h_avg * delta_hu / c_avg) / h_avg;
    
    // Correction pour éviter alpha1, alpha2 infinis
    if (h_avg < 1e-10)
    {
        alpha1 = 0.0;
        alpha2 = 0.0;
    }
    else
    {
        alpha1 = ((delta_hu - delta_h * u_avg) + c_avg * delta_h) / (2.0 * c_avg);
        alpha2 = (-(delta_hu - delta_h * u_avg) + c_avg * delta_h) / (2.0 * c_avg);
    }
    
    // 8. Calculer la contribution de chaque onde au flux
    // Vecteur propre 1 : r1 = (1, u_avg - c_avg)
    // Vecteur propre 2 : r2 = (1, u_avg + c_avg)
    
    double contrib_h = fabs(lambda1) * alpha1 + fabs(lambda2) * alpha2;
    double contrib_hu = fabs(lambda1) * alpha1 * (u_avg - c_avg) 
                      + fabs(lambda2) * alpha2 * (u_avg + c_avg);
    
    // 9. Flux de Roe : moyenne des flux - dissipation basée sur les ondes
    flux_h = 0.5 * (FL_h + FR_h) - 0.5 * contrib_h;
    flux_hu = 0.5 * (FL_hu + FR_hu) - 0.5 * contrib_hu;
    
    // Note : Le flux de Roe peut devenir instable dans certains cas
    // (présence de "sonic points"). On pourrait ajouter une correction
    // d'entropie (entropy fix) mais on garde simple ici.
}
// Plus précis que LF/Rusanov car il tient compte de la direction des ondes
// Formule :
//   - Si S_L >= 0 : F_HLL = F_L (tout va vers la droite)
//   - Si S_R <= 0 : F_HLL = F_R (tout va vers la gauche)
//   - Sinon : F_HLL = (S_R*F_L - S_L*F_R + S_L*S_R*(W_R - W_L)) / (S_R - S_L)
// 
// où S_L et S_R sont les vitesses d'onde minimale et maximale
// ========================================
void SaintVenant1D::FluxHLL(double hL, double huL, double hR, double huR,
                            double& flux_h, double& flux_hu)
{
    // 1. Calculer les flux physiques à gauche et à droite
    double FL_h, FL_hu;   // Flux gauche
    double FR_h, FR_hu;   // Flux droite
    
    CalculerFluxPhysique(hL, huL, FL_h, FL_hu);
    CalculerFluxPhysique(hR, huR, FR_h, FR_hu);
    
    // 2. Calculer les vitesses et célérités
    double uL = CalculerVitesse(hL, huL);
    double uR = CalculerVitesse(hR, huR);
    
    double cL = 0.0, cR = 0.0;
    if (hL > 1e-10) cL = sqrt(_g * hL);  // Célérité gauche
    if (hR > 1e-10) cR = sqrt(_g * hR);  // Célérité droite
    
    // 3. Estimer les vitesses d'onde S_L et S_R
    // Plusieurs choix possibles, on utilise les estimations classiques :
    // S_L = min(u_L - c_L, u_R - c_R)  (vitesse d'onde la plus à gauche)
    // S_R = max(u_L + c_L, u_R + c_R)  (vitesse d'onde la plus à droite)
    double S_L = min(uL - cL, uR - cR);
    double S_R = max(uL + cL, uR + cR);
    
    // 4. Calculer le flux HLL selon les cas
    
    // CAS 1 : Toutes les ondes vont vers la droite (S_L >= 0)
    // => On prend le flux de gauche
    if (S_L >= 0.0)
    {
        flux_h = FL_h;
        flux_hu = FL_hu;
    }
    // CAS 2 : Toutes les ondes vont vers la gauche (S_R <= 0)
    // => On prend le flux de droite
    else if (S_R <= 0.0)
    {
        flux_h = FR_h;
        flux_hu = FR_hu;
    }
    // CAS 3 : Les ondes vont dans les deux directions
    // => On fait une moyenne pondérée par les vitesses d'onde
    else
    {
        // Formule HLL : mélange des deux flux
        flux_h = (S_R * FL_h - S_L * FR_h + S_L * S_R * (hR - hL)) / (S_R - S_L);
        flux_hu = (S_R * FL_hu - S_L * FR_hu + S_L * S_R * (huR - huL)) / (S_R - S_L);
    }
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
    
    if (v_max > 1e-10)
        _dt = _CFL * _dx / v_max;
    else
        _dt = 0.01;  // Valeur par défaut si v_max = 0
}

// ========================================
// Avancer d'un pas de temps
// Schéma de Godunov avec flux numérique choisi
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
        CalculerFluxNumerique(_h[i], _hu[i], _h[i+1], _hu[i+1],
                              flux_droite_h, flux_droite_hu);
        
        // Flux à l'interface gauche (entre i-1 et i)
        double flux_gauche_h, flux_gauche_hu;
        CalculerFluxNumerique(_h[i-1], _hu[i-1], _h[i], _hu[i],
                              flux_gauche_h, flux_gauche_hu);
        
        // Mise à jour conservative :
        // W_nouveau = W_ancien - (dt/dx) * (Flux_droite - Flux_gauche)
        double coeff = _dt / _dx;
        h_nouveau[i] = _h[i] - coeff * (flux_droite_h - flux_gauche_h);
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