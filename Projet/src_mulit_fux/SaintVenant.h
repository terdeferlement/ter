#ifndef _SAINT_VENANT_H
#define _SAINT_VENANT_H

#include <vector>
#include <string>
#include <fstream>

// ========================================
// Énumération pour choisir le type de flux
// ========================================
enum TypeFlux
{
    FLUX_LAX_FRIEDRICHS,  // Flux de Lax-Friedrichs (alpha global)
    FLUX_RUSANOV,          // Flux de Rusanov (alpha local)
    FLUX_HLL,              // Flux HLL (plus précis)
    FLUX_ROE               // Flux de Roe (le plus précis)
};

// ========================================
// Classe principale : résout Saint-Venant 1D
// ========================================
class SaintVenant1D
{
private:
    // Paramètres du domaine
    int _N;              // Nombre de cellules
    double _L;           // Longueur du domaine [0, L]
    double _dx;          // Pas d'espace : dx = L/N
    
    // Paramètres temporels
    double _t;           // Temps actuel
    double _dt;          // Pas de temps
    double _CFL;         // Nombre CFL (< 0.5 pour stabilité)
    
    // Variables de la solution
    // W = (h, hu) où h = hauteur, hu = débit
    std::vector<double> _h;   // Hauteur d'eau dans chaque cellule
    std::vector<double> _hu;  // Débit (h*u) dans chaque cellule
    
    // Constante physique
    static constexpr double _g = 9.81;  // Gravité (m/s²)
    
    // Type de flux numérique à utiliser
    TypeFlux _type_flux;
    
    // Fichier pour sauvegarder
    std::ofstream _fichier;

public:
    // Constructeur
    SaintVenant1D();
    
    // Destructeur
    ~SaintVenant1D();
    
    // Initialiser la simulation
    void Initialiser(int N, double L, double CFL, std::string nom_fichier, 
                     TypeFlux type_flux = FLUX_LAX_FRIEDRICHS);
    
    // Définir la condition initiale
    void ConditionInitialeBarrage(double x_barrage);
    
    // Calculer le flux physique F(h, hu) = (hu, hu²/h + g*h²/2)
    void CalculerFluxPhysique(double h, double hu, double& F_h, double& F_hu);
    
    // Calculer le flux numérique entre deux cellules (selon le type choisi)
    void CalculerFluxNumerique(double hL, double huL, double hR, double huR,
                               double& flux_h, double& flux_hu);
    
    // Flux de Lax-Friedrichs (version originale avec alpha GLOBAL)
    void FluxLaxFriedrichs(double hL, double huL, double hR, double huR,
                           double& flux_h, double& flux_hu);
    
    // Flux de Rusanov (avec alpha LOCAL à l'interface)
    void FluxRusanov(double hL, double huL, double hR, double huR,
                     double& flux_h, double& flux_hu);
    
    // Flux HLL (Harten-Lax-van Leer)
    void FluxHLL(double hL, double huL, double hR, double huR,
                 double& flux_h, double& flux_hu);
    
    // Flux de Roe (linéarisation exacte)
    void FluxRoe(double hL, double huL, double hR, double huR,
                 double& flux_h, double& flux_hu);
    
    // Calculer la vitesse u = hu/h
    double CalculerVitesse(double h, double hu);
    
    // Calculer la vitesse maximale (pour le pas de temps CFL)
    double VitesseMaximale();
    
    // Calculer le pas de temps avec la condition CFL
    void CalculerPasDeTemps();
    
    // Avancer d'un pas de temps (schéma de Godunov)
    void Avancer();
    
    // Sauvegarder la solution dans le fichier
    void Sauvegarder();
    
    // Accesseurs
    double ObtenirTemps() const { return _t; }
    double ObtenirDt() const { return _dt; }
    std::string ObtenirNomFlux() const;
};

#endif // _SAINT_VENANT_H